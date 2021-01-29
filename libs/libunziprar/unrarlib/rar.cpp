#include "rar.hpp"


#include "smallfn.cpp"

#include <pspkernel.h>
#include <pspthreadman.h>
#include <pspdebug.h>
#include <psppower.h>
#include <pspiofilemgr.h>
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

//PSP_MODULE_INFO("AnTi Unzip", 0, 1, 1);
//PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#include "rarmainH.h"

#ifdef _DJGPP
extern "C" char **__crt0_glob_function (char *arg) { return 0; }
extern "C" void   __crt0_load_environment_file (char *progname) { }
#endif

//#if !defined(GUI) && !defined(RARDLL)
extern "C" int mainRAR(int argc, char *argv[])
{
//pspDebugScreenInit();
    dummy_printf("extractingRar");
//SceUID handle;
//handle=sceIoOpen("ms0:/test.rar",O_RDWR | O_CREAT,0777);
//dummy_printf("OPened%s","ms0:/test.rar");
//if(handle < 0){dummy_printf("ErrOr!!");handle = BAD_HANDLE;}
//sceIoClose(handle);

#ifdef _UNIX
  setlocale(LC_ALL,"");
#endif
#if defined(_EMX) && !defined(_DJGPP)
  uni_init(0);
#endif
#ifndef SFX_MODULE
  setbuf(stdout,NULL);

  #ifdef _EMX
    EnumConfigPaths(argv[0],-1);
  #endif
#endif

  ErrHandler.SetSignalHandlers(true);
  dummy_printf(".");
  RARInitData();
  dummy_printf(".");

#ifdef SFX_MODULE
  char ModuleName[NM];
#ifdef _WIN_32
  GetModuleFileName(NULL,ModuleName,sizeof(ModuleName));
#else
  strcpy(ModuleName,argv[0]);
#endif
#endif

#ifdef _WIN_32
  SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);


#endif
dummy_printf(".");

#ifdef ALLOW_EXCEPTIONS
  try 
#endif
  {
  dummy_printf(".");
    CommandData Cmd;
    Cmd.Init();
#ifdef SFX_MODULE
    strcpy(Cmd.Command,"X");
    char *Switch=NULL;
#ifdef _SFX_RTL_
    char *CmdLine=GetCommandLine();
    if (CmdLine!=NULL && *CmdLine=='\"')
      CmdLine=strchr(CmdLine+1,'\"');
    if (CmdLine!=NULL && (CmdLine=strpbrk(CmdLine," /"))!=NULL)
    {
      while (isspace(*CmdLine))
        CmdLine++;
      Switch=CmdLine;
    }
#else
    dummy_printf(".");
    Switch=argc>1 ? argv[1]:NULL;
#endif
    if (Switch!=NULL && Cmd.IsSwitch(Switch[0]))
    {
      int UpperCmd=toupper(Switch[1]);
      switch(UpperCmd)
      {
        case 'T':
        case 'V':
          Cmd.Command[0]=UpperCmd;
          break;
        case '?':
          Cmd.OutHelp();
          break;
      }
    }
    dummy_printf(".");
    Cmd.AddArcName(ModuleName,NULL);
    dummy_printf(".");
#else
    //seems to die here second time round
    
    if (Cmd.IsConfigEnabled(argc,argv))
    {
      Cmd.ReadConfig(argc,argv);
      Cmd.ParseEnvVar();
    }
    for (int I=1;I<argc;I++)
      Cmd.ParseArg(argv[I],NULL);
    
    dummy_printf(".");
#endif
    dummy_printf(".");
    Cmd.ParseDone();

    dummy_printf(".");
    InitConsoleOptions(Cmd.MsgStream,Cmd.Sound);
    InitLogOptions(Cmd.LogName);
    ErrHandler.SetSilent(Cmd.AllYes || Cmd.MsgStream==MSG_NULL);
    ErrHandler.SetShutdown(Cmd.Shutdown);

    Cmd.OutTitle();
    Cmd.ProcessCommand();
    Cmd.Close();
    dummy_printf(".");
  }
  
#ifdef ALLOW_EXCEPTIONS
  catch (int ErrCode)
  {
    ErrHandler.SetErrorCode(ErrCode);
  }
#ifdef ENABLE_BAD_ALLOC
  catch (bad_alloc)
  {
    ErrHandler.SetErrorCode(MEMORY_ERROR);
  }
#endif
  catch (...)
  {
    ErrHandler.SetErrorCode(FATAL_ERROR);
  }
#endif
  File::RemoveCreated();
#if defined(SFX_MODULE) && defined(_DJGPP)
  //CODE BRUTALLY CHANGED
  //_chmod(ModuleName,1,0x20);
#endif
#if defined(_EMX) && !defined(_DJGPP)
  uni_done();
#endif
  return(ErrHandler.GetErrorCode());
      dummy_printf("Complete");
}


