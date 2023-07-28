/*
   miniunz.c
   Version 1.01b, May 30th, 2004

   Copyright (C) 1998-2004 Gilles Vollant
*/
#define unix

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <pspiofilemgr.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <stdio.h>

#define printf    dummy_printf//dummy_printf 
#include "unzip.h"

#ifdef unix
# include <unistd.h>
# include <utime.h>
#else
# include <direct.h>
# include <io.h>
#endif

#include "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif
/*
  mini unzip, demo of unzip package

  usage :
  Usage : miniunz [-exvlo] file.zip [file_to_extract] [-d extractdir]

  list the file in the zipfile, and print the content of FILE_ID.ZIP or README.TXT
    if it exists
*/


/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */
void change_file_date(filename,dosdate,tmu_date)
    const char *filename;
    uLong dosdate;
    tm_unz tmu_date;
{
#ifdef WIN32
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#else
#ifdef unix
  /*
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
  */
#endif
#endif
}


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

int usePassword = 0;

void SetUsepassword(int p){usePassword = p;}
int GetUsePassword(){return usePassword;}

int mymkdir(dirname)
    const char* dirname;
{
    int ret=0;
#ifdef WIN32
    ret = mkdir(dirname);
#else
#ifdef unix
    ret = sceIoMkdir(dirname,0777);
    //ret = mkdir (dirname,0775);
#endif
#endif
    return ret;
}

int makedir (newdir)
    char *newdir;
{
  char *buffer ;
  char *p;
  int  len = (int)strlen(newdir);

  if (len <= 0)
    return 0;

  buffer = (char*)malloc(len+1);
  strcpy(buffer,newdir);

  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
}


void do_banner()
{
    printf("AnonymousTipster's .zip plugin, based on:\n");
    printf("MiniUnz 1.01b, demo of zLib + Unz package written by Gilles Vollant\n");
    printf("more info at http://www.winimage.com/zLibDll/unzip.html\n\n");
}

void do_help()
{
    printf("Usage : miniunz [-e] [-x] [-v] [-l] [-o] [-p password] file.zip [file_to_extr.] [-d extractdir]\n\n" \
           "  -e  Extract without pathname (junk paths)\n" \
           "  -x  Extract with pathname\n" \
           "  -v  list files\n" \
           "  -l  list files\n" \
           "  -d  directory to extract into\n" \
           "  -o  overwrite files without prompting\n" \
           "  -p  extract crypted file using password\n\n");
}


int do_list(uf)
    unzFile uf;
{
    uLong i;
    unz_global_info gi;
    int err;

    err = unzGetGlobalInfo (uf,&gi);
    if (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);
    printf(" Length  Method   Size  Ratio   Date    Time   CRC-32     Name\n");
    printf(" ------  ------   ----  -----   ----    ----   ------     ----\n");
    for (i=0;i<gi.number_entry;i++)
    {
        char filename_inzip[256];
        unz_file_info file_info;
        uLong ratio=0;
        const char *string_method;
        char charCrypt=' ';
        err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
            break;
        }
        if (file_info.uncompressed_size>0)
            ratio = (file_info.compressed_size*100)/file_info.uncompressed_size;

        /* display a '*' if the file is crypted */
        if ((file_info.flag & 1) != 0)
            charCrypt='*';

        if (file_info.compression_method==0)
            string_method="Stored";
        else
        if (file_info.compression_method==Z_DEFLATED)
        {
            uInt iLevel=(uInt)((file_info.flag & 0x6)/2);
            if (iLevel==0)
              string_method="Defl:N";
            else if (iLevel==1)
              string_method="Defl:X";
            else if ((iLevel==2) || (iLevel==3))
              string_method="Defl:F"; /* 2:fast , 3 : extra fast*/
        }
        else
            string_method="Unkn. ";

        printf("%7lu  %6s%c%7lu %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
                file_info.uncompressed_size,string_method,
                charCrypt,
                file_info.compressed_size,
                ratio,
                (uLong)file_info.tmu_date.tm_mon + 1,
                (uLong)file_info.tmu_date.tm_mday,
                (uLong)file_info.tmu_date.tm_year % 100,
                (uLong)file_info.tmu_date.tm_hour,(uLong)file_info.tmu_date.tm_min,
                (uLong)file_info.crc,filename_inzip);
        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}


int do_extract_currentfile(uf,popt_extract_without_path,popt_overwrite,password,destpath)
    unzFile uf;
    const int* popt_extract_without_path;
    int* popt_overwrite;
    const char* password;
    const char* destpath;
{
    char filename_inzip[256], cache[256];
    char abs_filename[512];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fot=NULL;
    SceUID fout=NULL;
    u8* buf;
    uInt size_buf;
    strcpy(abs_filename, destpath);

    unz_file_info file_info;
    uLong ratio=0;
    err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
    
    int i;
    for (i = 0; filename_inzip[i] != '\0'; i++)
        cache[i] = (char)tolower(filename_inzip[i]);
    cache[i] = '\0';
    
    //printf("cache : %s\n",cache);
    /*
    char *eIdx = strstr(cache, "eboot.pbp");
    if ( eIdx != NULL )
    {
        int idx = eIdx - cache;
        //printf("Transforming %s, idx= %i\n",filename_inzip,idx);
        char nname[] = "VBOOT.PBP";
        
        for ( i=0; i<strlen(nname); ++i )
            filename_inzip[idx+i] = nname[i];
    }
    */
    
    strcat(abs_filename, filename_inzip);
    

    if (err!=UNZ_OK)
    {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)memalign(64,size_buf);
    if (buf==NULL)
    {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0')
    {
        if ((*popt_extract_without_path)==0)
        {
            char* end = abs_filename + strlen(abs_filename) -1;
            if ( end[0] == '\\' || end[0] == '/' )    end[0] = '\0';
            printf("creating directory: %s\n",abs_filename);
            mymkdir(abs_filename);
        }
    }
    else
    {
        const char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = abs_filename;
        else
            write_filename = filename_withoutpath;

        if(usePassword == 1){
            err = unzOpenCurrentFilePassword(uf,password);}else{err = unzOpenCurrentFilePassword(uf,NULL);}
        if (err!=UNZ_OK)
        {
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);
        }

        if (((*popt_overwrite)==0) && (err==UNZ_OK))
        {
            char rep=0;
            FILE* ftestexist;
            ftestexist = fopen(write_filename,"rb");
            if (ftestexist!=NULL)
            {
                fclose(ftestexist);
                do
                {
                    char answer[128];
                    int ret;

                    printf("The file %s will be overwritten ",write_filename);
                    ret = 'y';//scanf("%1s",answer);
                    if (ret != 1)
                    {
                       //exit(EXIT_FAILURE);
                        free(buf);
                        return UNZ_INTERNALERROR;
                    }
                    rep = answer[0] ;
                    if ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                }
                while ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            if (rep == 'N')
                skip = 1;

            if (rep == 'A')
                *popt_overwrite=1;
        }

        if ((skip==0) && (err==UNZ_OK))
        {
            printf("fopen %s\n",write_filename);
            fot=fopen(write_filename,"wb");

            /* some zipfile don't contain directory alone before file */
            if ((fot==NULL) && ((*popt_extract_without_path)==0) &&
                                (filename_withoutpath!=(char*)filename_inzip))
            {
                char c=*(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                printf("makedir %s\n",write_filename);
                //sceKernelExitGame();
                makedir(write_filename);
                *(filename_withoutpath-1)=c;
                fot=fopen(write_filename,"wb");
            }

            if (fot==NULL)
            {
                printf("error opening %s\n",write_filename);
            }
        }
        fclose(fot);
        fout = sceIoOpen(write_filename,O_RDWR | O_CREAT,0777);
        int counter;counter = 0;
        if (fout!=NULL)
        {
            //printf(" extracting: %s\n",write_filename);

            do
            {
                //printf(".%i",counter);
                //pspDebugScreenSetXY(0,4);
                printf(" extracting: %s(%i)\n",write_filename,counter);
                counter++;
                err = unzReadCurrentFile(uf,buf,size_buf);
                if (err<0)
                {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }//sceIoWrite(fout,buf,err);
                if (err>0)
                    if (sceIoWrite(fout,buf,err)<1)
                    {//fwrite(buf,err,1,fout)
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
            }
            while (err>0);
            if (fout)
                    sceIoClose(fout);

            if (err==0)
                change_file_date(write_filename,file_info.dosDate,
                                 file_info.tmu_date);
        }

        if (err==UNZ_OK)
        {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
            }
        }
        else
            unzCloseCurrentFile(uf); /* don't lose the error */
    }

    free(buf);
    return err;
}


int do_extract(uf,opt_extract_without_path,opt_overwrite,password,destpath)
    unzFile uf;
    int opt_extract_without_path;
    int opt_overwrite;
    const char* password;
    const char* destpath;
{
    uLong i;
    unz_global_info gi;
    int err;
    FILE* fout=NULL;

    err = unzGetGlobalInfo (uf,&gi);
    if (err!=UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);

    for (i=0;i<gi.number_entry;i++)
    {
        if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password,destpath) != UNZ_OK)
            break;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

int do_extract_onefile(uf,filename,opt_extract_without_path,opt_overwrite,password)
    unzFile uf;
    const char* filename;
    int opt_extract_without_path;
    int opt_overwrite;
    const char* password;
{
    int err = UNZ_OK;
    if (unzLocateFile(uf,filename,CASESENSITIVITY)!=UNZ_OK)
    {
        printf("file %s not found in the zipfile\n",filename);
        return 2;
    }

    if (do_extract_currentfile(uf,&opt_extract_without_path,
                                      &opt_overwrite,
                                      password,"") == UNZ_OK)
        return 0;
    else
        return 1;
}


int unzipToDir(const char *zippath, const char *destpath, const char *pass)
{
    const char *zipfilename=NULL;
    const char *filename_to_extract=NULL;
    const char *password=NULL;
    char filename_try[MAXFILENAME+16] = "";
    int i;
    int opt_do_list=0;
    int opt_do_extract=1;
    int opt_do_extract_withoutpath=0;
    int opt_overwrite=1;
    int opt_extractdir=1;
    const char *dirname=NULL;
    unzFile uf=NULL;

    do_banner();
    /*
    if (argc==1)
    {
        do_help();
        return 0;
    }
    else
    {
        for (i=1;i<argc;i++)
        {
            if ((*argv[i])=='-')
            {
                const char *p=argv[i]+1;

                while ((*p)!='\0')
                {
                    char c=*(p++);;
                    if ((c=='l') || (c=='L'))
                        opt_do_list = 1;
                    if ((c=='v') || (c=='V'))
                        opt_do_list = 1;
                    if ((c=='x') || (c=='X'))
                        opt_do_extract = 1;
                    if ((c=='e') || (c=='E'))
                        opt_do_extract = opt_do_extract_withoutpath = 1;
                    if ((c=='o') || (c=='O'))
                        opt_overwrite=1;
                    if ((c=='d') || (c=='D'))
                    {
                        opt_extractdir=1;
                        dirname=argv[i+1];
                    }

                    if (((c=='p') || (c=='P')) && (i+1<argc))
                    {
                        password=argv[i+1];
                        i++;
                    }
                }
            }
            else
            {
                if (zipfilename == NULL)
                    zipfilename = argv[i];
                else if ((filename_to_extract==NULL) && (!opt_extractdir))
                        filename_to_extract = argv[i] ;
            }
        }
    }*/
    zipfilename = zippath;//"ms0:/testzip.zip";
    ////filename_to_extract = "ms0:/zext/";
    dirname = destpath;//"ms0:/zext/";
    password = pass;

    //make destpath, so we don't try to extract to nonexistant
    makedir(dirname);

    if (zipfilename!=NULL)
    {

#        ifdef USEWIN32IOAPI
        zlib_filefunc_def ffunc;
#        endif

        strncpy(filename_try, zipfilename,MAXFILENAME-1);
        /* strncpy doesnt append the trailing NULL, of the string is too long. */
        filename_try[ MAXFILENAME ] = '\0';

#        ifdef USEWIN32IOAPI
        fill_win32_filefunc(&ffunc);
        uf = unzOpen2(zipfilename,&ffunc);
#        else
        uf = unzOpen(zipfilename);
#        endif
        if (uf==NULL)
        {
            strcat(filename_try,".zip");
#            ifdef USEWIN32IOAPI
            uf = unzOpen2(filename_try,&ffunc);
#            else
            uf = unzOpen(filename_try);
#            endif
        }
    }

    if (uf==NULL)
    {
        printf("Cannot open %s or %s.zip\n",zipfilename,zipfilename);
        return 1;
    }
    printf("%s opened\n",filename_try);

    if (opt_do_list==1)
        return do_list(uf);
    else if (opt_do_extract==1)
    {
        /*
        if (opt_extractdir && chdir(dirname))
        {
          printf("Error changing into %s, aborting\n", dirname);
          exit(-1);
        }
        */

        if (filename_to_extract == NULL)
            return do_extract(uf,opt_do_extract_withoutpath,opt_overwrite,password,dirname);
        else
            return do_extract_onefile(uf,filename_to_extract,
                                      opt_do_extract_withoutpath,opt_overwrite,password);
    }
    unzCloseCurrentFile(uf);

    return 0;
}

void testPrint(){
    printf("hello!");
}