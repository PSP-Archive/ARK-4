#ifndef _RAR_RARCOMMON_
#define _RAR_RARCOMMON_

#include "raros.hpp"
#include "os.hpp"

#define LITTLE_ENDIAN 

#include <string> 
#include <algorithm>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>

#include <pspdebug.h>
#include <psppower.h>
#include <pspiofilemgr.h>
#include <pspstdio.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <time.h> 
#include <malloc.h>
/*
#define mprintf	dummy_printf
#define printf	dummy_printf
#define pspDebugScreenPrintf	dummy_printf*/
#include "../unzip.h"

/*
#define FILE* SceUID 
#define FILE * SceUID
#define fread(b,z,n,s) sceIoRead(s,b,z*n) 
#define fopen(f,m) sceIoOpen(f,O_RDWR | O_CREAT,0777) 
#define fwrite(b,z,n,s) sceIoWrite(s,b,z*n) 
#define fclose(s) sceIoClose(s) 
#define ftell(s) sceIoLseek(s, 0, SEEK_CUR);
#define ferror(s) 0
#define clearerr(s) 0

#define fflush(s)  //NULL
  */

#ifdef RARDLL
#include "dll.hpp"
#endif

#ifndef _WIN_CE
#include "version.hpp"
#endif
#include "rartypes.hpp"
#include "rardefs.hpp"
#include "rarlang.hpp"
#include "int64.hpp"
#include "unicode.hpp"
#include "errhnd.hpp"
#include "array.hpp"
#include "timefn.hpp"
#include "headers.hpp"
#include "rarfn.hpp"
#include "pathfn.hpp"
#include "strfn.hpp"
#include "strlist.hpp"
#include "file.hpp"
#include "sha1.hpp"
#include "crc.hpp"
#include "rijndael.hpp"
#include "crypt.hpp"
#include "filefn.hpp"
#include "filestr.hpp"
#include "find.hpp"
#include "scantree.hpp"
#include "savepos.hpp"
#include "getbits.hpp"
#include "rdwrfn.hpp"
#include "options.hpp"
#include "archive.hpp"
#include "match.hpp"
#include "cmddata.hpp"
#include "filcreat.hpp"
#include "consio.hpp"
#include "system.hpp"
#include "isnt.hpp"
#include "log.hpp"
#include "rawread.hpp"
#include "encname.hpp"
#include "resource.hpp"
#include "compress.hpp"


#include "rarvm.hpp"
#include "model.hpp"


#include "unpack.hpp"


#include "extinfo.hpp"
#include "extract.hpp"



#include "list.hpp"



#include "rs.hpp"
#include "recvol.hpp"
#include "volume.hpp"
#include "smallfn.hpp"
#include "ulinks.hpp"

#include "global.hpp"


#define O_RDONLY	0x0001
#define O_WRONLY	0x0002
#define O_RDWR	(O_RDONLY | O_WRONLY)
#define O_NBLOCK	0x0004
#define O_DIROPEN	0x0008	// Internal use for dopen
#define O_APPEND	0x0100
#define O_CREAT	0x0200
#define O_TRUNC	0x0400
#define	O_EXCL	0x0800
#define O_NOWAIT	0x8000

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

/*

int mymkdir(const char*dirname)
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

int mymakedir(const char *newdir)
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

char* stripToSlash(char* string){
int i=0;
int len=0;
len = strlen(string);
for(i=len;i>0;i--){
	if(string[i] == '/'){break;}else{string[i] = '\0';}
}
return string;
}
*/
#endif
