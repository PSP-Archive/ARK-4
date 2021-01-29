#include <stdio.h>


void DoExtractRAR(const char *rarfile,const char *extDir,const char *pass){
    
makedir(extDir);
int argc;int i=0;
const char **argv = calloc ( 8, sizeof(char *) );
//argv = (char**)memalign(16,512);
int count;
const char ** currentString = argv;
if(pass == NULL){
argc = 5;
count = argc+1;

for ( i = 0; i < count; i++ )
  {
    if(i==0)asprintf ( currentString, "unrar");
    if(i==1)asprintf ( currentString, "x");
    if(i==2)asprintf ( currentString, "%s",rarfile);
    if(i==3)asprintf ( currentString, "%s",extDir);
    if(i==4)asprintf ( currentString, "-y");
    if(i> 4)asprintf ( currentString, " ");
    currentString++;
    //printf("currentString address: %p\n", currentString);  
  }
}else{

argc = 6;
count = argc+1;

for ( i = 0; i < count; i++ )
  {
    if(i==0)asprintf ( currentString, "unrar");
    if(i==1)asprintf ( currentString, "x");
    if(i==2)asprintf ( currentString, "%s",rarfile);
    if(i==3)asprintf ( currentString, "%s",extDir);
    if(i==4)asprintf ( currentString, "-y");
    if(i==5)asprintf ( currentString, "-p%s",pass);
    if(i> 5)asprintf ( currentString, " ");
    currentString++;
    //printf("currentString address: %p\n", currentString);  
  }

}
  // reset memory block to the original address.  
  // In other words, go the beginning of the "array"
  currentString = argv;
  //printf("currentString address after reset: %p\n", currentString);
  

  // display the string at this particular slot.
  // we have to use the star to de-reference
  //for ( i = 0; i < count; i++ )
  //{
  // dummy_printf( "%s\n", *currentString );
  //  currentString++;
  //}

    //argv[0] = "unrar";
    //argv[1] = "x";
    //argv[2] = rarfile;
    //argv[3] = extDir;
    //argv[4] = "-y";
    mainRAR(argc,argv);

  // reset
  currentString = argv;


for ( i = 0; i < count; i++ )
  {
    free ( *currentString );
    currentString++;
  }
free(argv);


}

int isRar(const char* fl){
int last;last = strlen(fl)-1;
if(fl[last] == 'r' && fl[last-1] == 'a' && fl[last-2] == 'r' && fl[last-3] == '.'){return(1);}
if(fl[last] == 'R' && fl[last-1] == 'A' && fl[last-2] == 'R' && fl[last-3] == '.'){return(1);}
if(fl[last] == 'a' && fl[last-1] == 'a' && fl[last-2] == 'R' && fl[last-3] == '.'){return(1);}
return(0);
}

