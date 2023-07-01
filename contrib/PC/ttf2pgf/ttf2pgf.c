/* This file is hereby licensed under the GNU General Public License.
   See COPYING for details. */

/* skylark@mips.for.ever */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYNTHESIS_H

#include "header.h"
#include "magic.h"

int face_size=24;
int face_embolden=0;
int face_italicize=0;
float face_hscale=1.0;
float face_advscale=1.0;

int shadow_do=1;
float shadow_blur=2.0;
float shadow_shade=0.8;

void parseface(char *s)
{
	char *e;
	for(;*s;)
		switch(*s) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			face_size=strtol(s,&e,0);
			if(face_size<8) {
				fprintf(stderr,"Face size cannot be less than 8.\n");
				exit(1);
			}
			s=e;
			break;
		case 'b':
			face_embolden=1;
			s++;
			break;
		case 'i':
			face_italicize=1;
			s++;
			break;
		case 'h':
			s++;
			face_hscale=strtod(s,&e);
			if(face_hscale<0.25) {
				fprintf(stderr,"Face H-scale cannot be less than 0.25.\n");
				exit(1);
			}
			if(face_hscale>2.0) {
				fprintf(stderr,"Face H-scale cannot be greater than 2.0.\n");
				exit(1);
			}
			s=e;
			break;
		case 'a':
			s++;
			face_advscale=strtod(s,&e);
			if(face_advscale<0.25) {
				fprintf(stderr,"Face advance-scale cannot be less than 0.25.\n");
				exit(1);
			}
			if(face_advscale>2.0) {
				fprintf(stderr,"Face advance-scale cannot be greater than 2.0.\n");
				exit(1);
			}
			s=e;
			break;
		default:
			fprintf(stderr,"Invalid face option '%c'.\n", *s);
			exit(1);
		}
}
void parseshadow(char *s)
{
	char *e;
	for(;*s;)
		switch(*s) {
		case 'n':
			shadow_do=0;
			s++;
			break;
		case 'b':
			s++;
			shadow_blur=strtod(s,&e);
			if(shadow_blur<0.1) {
				fprintf(stderr,"Shadow blur cannot be less than 0.1.\n");
				exit(1);
			}
			if(shadow_blur>4.0) {
				fprintf(stderr,"Shadow blur cannot be greater than 4.0.\n");
				exit(1);
			}
			s=e;
			break;
		case 'i':
			s++;
			shadow_shade=strtod(s,&e);
			if(shadow_shade<0.0) {
				fprintf(stderr,"Shadow intensity cannot be less than 0.0.\n");
				exit(1);
			}
			if(shadow_shade>4.0) {
				fprintf(stderr,"Shadow intensity cannot be greater than 4.0.\n");
				exit(1);
			}
			s=e;
			break;
		default:
			fprintf(stderr,"Invalid shadow option '%c'.\n", *s);
			exit(1);
		}
}

FT_Library ftlib;
FT_Face ftface;
FT_Face ftshfc;

void buildrle_add(char *x, int v, int *i)
{
	if(!x) {
		(*i)++;
		return;
	}
	if((*i)&1)
		x[(*i)/2]|=v<<4;
	else
		x[(*i)/2]|=v&15;
	(*i)++;
}
int buildrle_int(char *x, char *p, int l)
{
	int i=0,j=0,k;
	while(j<l) {
		for(k=j;(k<l)&&((k-j)<8);k++)
			if(p[k]!=p[j])
				break;
		k-=j;
		if(k>1) {	// smear
			buildrle_add(x,k-1,&i);
			buildrle_add(x,p[j],&i);
			j+=k;
		} else {	// copy
			for(k=j;(k<(l-1))&&((k-j)<8);k++)
				if(p[k]==p[k+1])
					break;
			if((k==l-1)&&((k-j)<8))
				k++;
			if(k==j)
				fprintf(stderr,"I AM NOT AMUSED\n");
			buildrle_add(x,16-(k-j),&i);
			for(;j<k;j++)
				buildrle_add(x,p[j],&i);
		}
	}
	return (i+1)/2;
}
char *buildrle(char *p, int l, int *s)
{
	char *x;
	*s=buildrle_int(NULL,p,l);
	x=calloc(*s,1);
	buildrle_int(x,p,l);
	return x;
}

void putv(int n, unsigned char *p, int *b, int v)
{
	int i;
	for(i=0;i<n;i++) {
		if(v&1)
			p[(*b)/8]|=1<<((*b)%8);
		else
			p[(*b)/8]&=~(1<<((*b)%8));
		v>>=1;
		(*b)++;
	}
}


struct vchar {
	int idx;
	short ch;
	/* input */
	char *bmp;
	int w,h;
	int d,a;
	int v;
	/* packed */
	int t;
	char *rle;
	int rlen;
	char *hdr;
	int hlen;
	/* shadow */
	struct vchar *sh;
};

void packchar(struct vchar *v)
{
	char *tbmp=malloc(v->w*v->h);
	char *trle;
	int trlen;
	int i,j;
	for(j=0;j<v->h;j++)
		for(i=0;i<v->w;i++)
			tbmp[i*v->h+j]=v->bmp[j*v->w+i];
	v->rle=buildrle(v->bmp,v->w*v->h,&(v->rlen));
	trle=buildrle(tbmp,v->w*v->h,&trlen);
	if(v->rlen>trlen) {
		free(v->rle);
		v->rle=trle;
		v->rlen=trlen;
		v->t=2;
	} else {
		free(trle);
		v->t=1;
	}
	free(tbmp);

	i=0;
	v->hlen=(v->idx>=0)?12:6;
	v->hdr=calloc(v->hlen,1);
	if(v->v>=35)
		v->v=35;
	putv(14,v->hdr,&i,v->rlen+v->hlen);
	putv(7,v->hdr,&i,v->w);
	putv(7,v->hdr,&i,v->h);
	putv(7,v->hdr,&i,v->d);
	putv(7,v->hdr,&i,v->a);
	putv(2,v->hdr,&i,v->t);
	if(v->hlen==12) {
		putv(1,v->hdr,&i,1);
		putv(4,v->hdr,&i,7);
		putv(6,v->hdr,&i,magic[v->idx]);
		putv(9,v->hdr,&i,v->idx);
		putv(8,v->hdr,&i,0x00);
		putv(8,v->hdr,&i,0x00);
		putv(8,v->hdr,&i,0x00);
		putv(2,v->hdr,&i,0x0);
		putv(6,v->hdr,&i,v->v);
	} else {
		putv(1,v->hdr,&i,0);
		putv(3,v->hdr,&i,0);
	}
	if(v->sh) {
		if(shadow_do)
			packchar(v->sh);
		else {
			v->sh->hdr=calloc(6,1);
			v->hlen=6;
		}
	}
}

float sqrf(float x)
{
	return x*x;
}

void blurpass(char *b, int w, int h, float r, float s)
{
	float *c=alloca(w*h*sizeof(float));
	float *d=alloca(w*h*sizeof(float));
	float x,rf=s/(r*1.77245);
	int i,j,t,m=(int)(2.0f*r+0.5f);
	float *k=alloca((2*m+1)*sizeof(float));
	for(j=0;j<h;j++)
		for(i=0;i<w;i++)
			c[j*w+i]=b[j*w+i];
	for(t=-m;t<=m;t++)
		k[t+m]=rf*expf(-sqrf(t/r));
	for(j=0;j<h;j++)
		for(i=0;i<w;i++) {
			x=0;
			for(t=-m;t<=m;t++)
				if(t+i>=0 && t+i<w)
					x+=c[j*w+(t+i)]*k[t+m];
			d[j*w+i]=x;
		}
	for(j=0;j<h;j++)
		for(i=0;i<w;i++) {
			x=0;
			for(t=-m;t<=m;t++)
				if(t+j>=0 && t+j<h)
					x+=d[(t+j)*w+i]*k[t+m];
			c[j*w+i]=x;
		}
	for(j=0;j<h;j++)
		for(i=0;i<w;i++) {
			c[j*w+i]+=0.5f;
			if(c[j*w+i]<0.0f)
				c[j*w+i]=0.0f;
			if(c[j*w+i]>15.0f)
				c[j*w+i]=15.0f;
			b[j*w+i]=(int)c[j*w+i];
		}
}

void loadchar(struct vchar *v)
{
	int gli=FT_Get_Char_Index(ftface, v->ch);
	int glj=FT_Get_Char_Index(ftshfc, v->ch);
	int i,j;
	if(FT_Load_Glyph(ftface, gli, FT_LOAD_DEFAULT)) {
		fprintf(stderr, "FreeType load glyph failed [F:U#%04X].\n", v->ch);
		exit(1);
	}
	if(face_embolden)
		FT_GlyphSlot_Embolden(ftface->glyph);
	if(face_italicize)
		FT_GlyphSlot_Oblique(ftface->glyph);
	if(FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_NORMAL)) {
		fprintf(stderr, "FreeType render glyph failed [F:U#%04X].\n", v->ch);
		exit(1);
	}
	if(FT_Load_Glyph(ftshfc, glj, FT_LOAD_DEFAULT)) {
		fprintf(stderr, "FreeType load glyph failed [S:U#%04X].\n", v->ch);
		exit(1);
	}
	if(face_embolden)
		FT_GlyphSlot_Embolden(ftshfc->glyph);
	if(face_italicize)
		FT_GlyphSlot_Oblique(ftshfc->glyph);
	if(FT_Render_Glyph(ftshfc->glyph, FT_RENDER_MODE_NORMAL)) {
		fprintf(stderr, "FreeType render glyph failed [S:U#%04X].\n", v->ch);
		exit(1);
	}
	v->w=ftface->glyph->bitmap.width;
	v->h=ftface->glyph->bitmap.rows;
	v->a=ftface->glyph->bitmap_top;
	v->d=ftface->glyph->bitmap_left;
	v->v=(int)(ftface->glyph->advance.x*face_advscale/32.0+0.5);
	if(!v->w || !v->h) {
		v->bmp=malloc(1);
		v->bmp[0]=0;
	} else {
		v->bmp=malloc(v->w*v->h);
		for(j=0;j<v->h;j++)
			for(i=0;i<v->w;i++)
				v->bmp[j*v->w+i]=ftface->glyph->bitmap.buffer[j*ftface->glyph->bitmap.pitch+i]>>4;
	}

	v->sh->w=ftshfc->glyph->bitmap.width+8;
	v->sh->h=ftshfc->glyph->bitmap.rows+8;
	v->sh->a=4+ftshfc->glyph->bitmap_top;
	v->sh->d=-4+ftshfc->glyph->bitmap_left;
	v->sh->bmp=calloc(v->sh->w,v->sh->h);
	for(j=0;j<v->sh->h-8;j++)
		for(i=0;i<v->sh->w-8;i++)
			v->sh->bmp[(j+4)*v->sh->w+(i+4)]=ftshfc->glyph->bitmap.buffer[j*ftshfc->glyph->bitmap.pitch+i]>>4;
	blurpass(v->sh->bmp,v->sh->w,v->sh->h,shadow_blur,shadow_shade);
}

struct vchar *createchars(int *n)
{
	int i;
	struct vchar *v;
	*n = *(int *)(fonthdr+0x14);
	v=calloc(*n,sizeof(struct vchar));
	for(i=0;i<*n;i++) {
		v[i].idx=i;
		v[i].ch=((short *)(fonthdr+0x1760))[i];
		v[i].sh=calloc(1,sizeof(struct vchar));
		v[i].sh->idx=-1;
		v[i].sh->ch=0;
		printf("%lc\n", v[i].ch);
		loadchar(v+i);
		packchar(v+i);
	}
	return v;
}

char *createptrs(struct vchar *v, int n, int *s, int *l)
{
	int i,ds=0,pl=0,j=0;
	char *p;
	for(i=0;i<n;i++) {
		ds+=v[i].rlen+v[i].hlen+v[i].sh->rlen+v[i].sh->hlen;
		ds=(ds+3)&~3;
	}
	while((4<<pl)<ds)
		pl++;
	*s=(((pl*n)+31)/32)*4;
	p=calloc(*s,1);
	ds=0;
	for(i=0;i<n;i++) {
		putv(pl,p,&j,ds/4);
		ds+=v[i].rlen+v[i].hlen+v[i].sh->rlen+v[i].sh->hlen;
		ds=(ds+3)&~3;
	}
	*l=pl;
	return p;
}

void fixupheader(char *n,char *s,int l)
{
	strcpy(fonthdr+0x35,n);
	strcpy(fonthdr+0x75,s);
	*(int *)(fonthdr+0x1C)=l;
}

void fwritez(int n,FILE *f)
{
	char x=0;
	while(n--)
		fwrite(&x,1,1,f);
}

void dumpHeader(){
	FILE* fp = fopen("header.bin", "w");
	fwrite(fonthdr, 1, sizeof(fonthdr), fp);
	fclose(fp);
}

int main(int argc, char *argv[])
{

	FILE *f;
	struct vchar *v;
	int n;
	char *p;
	int s,l;
	int i;
	FT_Matrix tm;
	char *ts;

	if(argc<3) {
		fprintf(stderr,"usage: ttf2pgf <source.ttf> <dest.pgf> [<face-options> [<shadow-options>]]\n");
		return 1;
	}

	if(FT_Init_FreeType(&ftlib)) {
		fprintf(stderr,"FreeType init failed.\n");
		return 1;
	}
	if(FT_New_Face(ftlib, argv[1], 0, &ftface) || FT_New_Face(ftlib, argv[1], 0, &ftshfc)) {
		fprintf(stderr,"FreeType load '%s' failed.\n", argv[1]);
		return 1;
	}
	
	FT_Select_Charmap(ftface, ft_encoding_unicode);
	FT_Select_Charmap(ftshfc, ft_encoding_unicode);
	
	if(argc>3)
		parseface(argv[3]);
	if(argc>4)
		parseshadow(argv[4]);

	if(FT_Set_Pixel_Sizes(ftface, 0, face_size)) {
		fprintf(stderr,"FreeType set pixel sizes failed [F].\n");
		return 1;
	}
	if(FT_Set_Pixel_Sizes(ftshfc, 0, face_size/2)) {
		fprintf(stderr,"FreeType set pixel sizes failed [S].\n");
		return 1;
	}

	if(face_hscale!=1.0) {
		tm.xx=(int)(0x10000*face_hscale);
		tm.xy=0;
		tm.yx=0;
		tm.yy=0x10000;
		FT_Set_Transform(ftface, &tm, NULL);
		FT_Set_Transform(ftshfc, &tm, NULL);
	}

	printf("create chars\n");
	v=createchars(&n);
	printf("create ptrs\n");
	p=createptrs(v,n,&s,&l);
	ts=ftface->style_name;
	if(face_embolden) {
		if(face_italicize)
			ts="Bold Italic";
		else
			ts="Bold";
	} else if(face_italicize)
		ts="Italic";
	printf("fix header\n");
	fixupheader(ftface->family_name,ts,l);

	printf("writing file\n");
	f=fopen(argv[2],"wb");
	fwrite(fonthdr,16504,1,f);
	fwrite(p,s,1,f);
	for(i=0;i<n;i++) {
		fwrite(v[i].hdr,v[i].hlen,1,f);
		fwrite(v[i].rle,v[i].rlen,1,f);
		fwrite(v[i].sh->hdr,v[i].sh->hlen,1,f);
		fwrite(v[i].sh->rle,v[i].sh->rlen,1,f);
		fwritez((4-((v[i].rlen+v[i].hlen+v[i].sh->rlen+v[i].sh->hlen)&3))&3,f);
	}
	fclose(f);
	return 0;
}
