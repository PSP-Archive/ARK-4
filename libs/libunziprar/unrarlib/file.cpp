#include "rar.hpp"

#define O_RDONLY    0x0001
#define O_WRONLY    0x0002
#define O_RDWR    (O_RDONLY | O_WRONLY)
#define O_NBLOCK    0x0004
#define O_DIROPEN    0x0008    // Internal use for dopen
#define O_APPEND    0x0100
#define O_CREAT    0x0200
#define O_TRUNC    0x0400
#define    O_EXCL    0x0800
#define O_NOWAIT    0x8000

#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

static File *CreatedFiles[256];
static int RemoveCreatedActive=0;

File::File()
{
  hFile=BAD_HANDLE;
  *FileName=0;
  *FileNameW=0;
  NewFile=false;
  LastWrite=false;
  HandleType=FILE_HANDLENORMAL;
  SkipClose=false;
  IgnoreReadErrors=false;
  ErrorType=FILE_SUCCESS;
  OpenShared=false;
  AllowDelete=true;
  CloseCount=0;
  AllowExceptions=true;
#ifdef _WIN_32
  NoSequentialRead=false;
#endif
}


File::~File()
{
  if (hFile!=BAD_HANDLE && !SkipClose)
    if (NewFile)
      Delete();
    else
      Close();
}


void File::operator = (File &SrcFile)
{
  hFile=SrcFile.hFile;
  strcpy(FileName,SrcFile.FileName);
  NewFile=SrcFile.NewFile;
  LastWrite=SrcFile.LastWrite;
  HandleType=SrcFile.HandleType;
  SrcFile.SkipClose=true;
}

int mymkdirR(char*dirname)
{
    printf("MakeDir:%s\n",dirname);
    int ret=0;
    ret = sceIoMkdir(dirname,0777);
    //ret = mkdir (dirname,0777);
    return ret;
}

int mymakedirR(char *newdir)
{
    //printf("CallmymakedirR");
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
  
  if (mymkdirR(buffer) == 0)
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
      if ((mymkdirR(buffer) == -1) && (errno == ENOENT))
        {
         dummy_printf("couldn't create directory %s\n",buffer);
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

char* stripToSlashR(char* string){
    //printf("CallStrip");
int i=0;
int len=0;
len = strlen(string);
for(i=len;i>0;i--){
    if(string[i] == '/'){break;}else{string[i] = '\0';}
}
return string;
}

bool File::Open(const char *Name,const wchar *NameW,bool OpenShared,bool Update)
{
    //printf("Open:%s",Name);
    /*
    printf("Open:%s",Name);
    //create dir if necessary
    char* tempname;tempname = (char*)memalign(16,512);
    sprintf(tempname,"%s",Name);
    mymakedirR(stripToSlashR(tempname));
    free(tempname);*/

  ErrorType=FILE_SUCCESS;
  SceUID hNewFile;
  if (File::OpenShared)
    OpenShared=true;
#ifdef _WIN_32
  uint Access=GENERIC_READ;
  if (Update)
    Access|=GENERIC_WRITE;
  uint ShareMode=FILE_SHARE_READ;
  if (OpenShared)
    ShareMode|=FILE_SHARE_WRITE;
  uint Flags=NoSequentialRead ? 0:FILE_FLAG_SEQUENTIAL_SCAN;
  if (WinNT() && NameW!=NULL && *NameW!=0)
    hNewFile=CreateFileW(NameW,Access,ShareMode,NULL,OPEN_EXISTING,Flags,NULL);
  else
    hNewFile=CreateFile(Name,Access,ShareMode,NULL,OPEN_EXISTING,Flags,NULL);

  if (hNewFile==BAD_HANDLE && GetLastError()==ERROR_FILE_NOT_FOUND)
    ErrorType=FILE_NOTFOUND;
#else
  int flags=Update ? O_RDWR | O_CREAT:O_RDONLY;
#ifdef O_BINARY
  //flags|=O_BINARY;
#if defined(_AIX) && defined(_LARGE_FILE_API)
  //flags|=O_LARGEFILE;
#endif
#endif
#if defined(_EMX) && !defined(_DJGPP)
  //int sflags=OpenShared ? SH_DENYNO:SH_DENYWR;
  //int handle=sopen(Name,flags,sflags);
#else
  //int handle=open(Name,flags);
  SceUID handle;
  handle=sceIoOpen((const char*)Name,flags,0777);
  //dummy_printf("Opened%s",(const char*)Name);
  if(handle < 0){/*dummy_printf("ERROR!!");*/sceIoClose(handle);handle = BAD_HANDLE;ErrorType=FILE_NOTFOUND;}
#ifdef LOCK_EX
  /*
  //CODE BRUTALLY REMOVED   && flock(handle,LOCK_EX|LOCK_NB)==-1
  if (!OpenShared && Update && handle>=0)
  {
    //close(handle);
      sceIoClose(handle);
    return(false);
  }*/
#endif
#endif
  //CODE BRUTALLY REMOVED
  //hNewFile=handle==-1 ? BAD_HANDLE:fdopen(handle,Update ? UPDATEBINARY:READBINARY);
  //if(handle){
  hNewFile=handle;
  //}
  //fits with if() below
  // && errno==ENOENT)
  if (hNewFile==0)
    ErrorType=FILE_NOTFOUND;
#endif
  NewFile=false;
  HandleType=FILE_HANDLENORMAL;
  SkipClose=false;
  bool Success=hNewFile!=BAD_HANDLE;
  if (Success)
  {
      //dummy_printf("Success!");
    hFile=hNewFile;
    if (NameW!=NULL)
      strcpyw(FileNameW,NameW);
    else
      *FileNameW=0;
    if (Name!=NULL)
      strcpy(FileName,Name);
    else
      WideToChar(NameW,FileName);
    AddFileToList(hFile);
  }
  return(Success);
}


#if !defined(SHELL_EXT) && !defined(SFX_MODULE)
void File::TOpen(const char *Name,const wchar *NameW)
{
  if (!WOpen(Name,NameW))
    ErrHandler.Exit(OPEN_ERROR);
}
#endif


bool File::WOpen(const char *Name,const wchar *NameW)
{
  if (Open(Name,NameW))
    return(true);
  ErrHandler.OpenErrorMsg(Name);
  return(false);
}


bool File::Create(const char *Name,const wchar *NameW)
{
    //sceIoMkdir("ms0:/Dae",0777);
    //printf("Create:%s",Name);
    //create dir if necessary
    char* tempname;tempname = (char*)memalign(16,512);
    char* tempname2;tempname2 = (char*)memalign(16,512);
    sprintf(tempname,"%s",Name);
    sprintf(tempname2,"%s",stripToSlashR(tempname));
    tempname2[strlen(tempname2)-1] = '\0';
    //printf("MakeDirC:%s\n",tempname2);
    mymakedirR(tempname2);
    free(tempname);
    free(tempname2);

#ifdef _WIN_32
  if (WinNT() && NameW!=NULL && *NameW!=0)
    hFile=CreateFileW(NameW,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,
                      CREATE_ALWAYS,0,NULL);
  else
    hFile=CreateFile(Name,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,
                     CREATE_ALWAYS,0,NULL);
#else
  //dummy_printf("Creating");
  //hFile=fopen(Name,CREATEBINARY);
  hFile=sceIoOpen(Name,O_RDWR | O_CREAT,0777);
  //sceIoClose(hFile);
#endif
  NewFile=true;
  HandleType=FILE_HANDLENORMAL;
  SkipClose=false;
  if (NameW!=NULL)
    strcpyw(FileNameW,NameW);
  else
    *FileNameW=0;
  if (Name!=NULL)
    strcpy(FileName,Name);
  else
    WideToChar(NameW,FileName);
  AddFileToList(hFile);
  return(hFile!=BAD_HANDLE);
}


void File::AddFileToList(SceUID hFile)
{
  if (hFile!=BAD_HANDLE)
    for (int I=0;I<sizeof(CreatedFiles)/sizeof(CreatedFiles[0]);I++)
      if (CreatedFiles[I]==NULL)
      {
        CreatedFiles[I]=this;
        break;
      }
}


#if !defined(SHELL_EXT) && !defined(SFX_MODULE)
void File::TCreate(const char *Name,const wchar *NameW)
{
  if (!WCreate(Name,NameW))
    ErrHandler.Exit(FATAL_ERROR);
}
#endif


bool File::WCreate(const char *Name,const wchar *NameW)
{
  if (Create(Name,NameW))
    return(true);
  ErrHandler.SetErrorCode(CREATE_ERROR);
  ErrHandler.CreateErrorMsg(Name);
  return(false);
}


bool File::Close()
{
  bool Success=true;
  if (HandleType!=FILE_HANDLENORMAL)
    HandleType=FILE_HANDLENORMAL;
  else
    if (hFile!=BAD_HANDLE)
    {
      if (!SkipClose)
      {
#ifdef _WIN_32
        Success=CloseHandle(hFile);
#else
        //dummy_printf("Closing");
        //dummy_printf("Closing:%s",this->FileName);
        Success=sceIoClose(hFile)>=0;
        //Success=0;//fclose(hFile)!=EOF;
#endif
        if (Success || !RemoveCreatedActive)
          for (int I=0;I<sizeof(CreatedFiles)/sizeof(CreatedFiles[0]);I++)
            if (CreatedFiles[I]==this)
            {
                //dummy_printf("DoneClose");
              CreatedFiles[I]=NULL;
              break;
            }
      }
      hFile=BAD_HANDLE;
      if (!Success && AllowExceptions)
        ErrHandler.CloseError(FileName);
    }
  CloseCount++;
  return(Success);
}


void File::Flush()
{
#ifdef _WIN_32
  FlushFileBuffers(hFile);
#else
  //CODE BRUTALLY REMOVED
  //fflush(hFile);
  //dummy_printf("flushB");
#endif
}


bool File::Delete()
{
    //dummy_printf("Delete");
  if (HandleType!=FILE_HANDLENORMAL || !AllowDelete)
    return(false);
  if (hFile!=BAD_HANDLE)
    Close();
  return(DelFile(FileName,FileNameW));
}


bool File::Rename(const char *NewName)
{
    //dummy_printf("Rename");
  bool Success=strcmp(FileName,NewName)==0;
  if (!Success)
    Success=sceIoRename(FileName,NewName)==0;
  if (Success)
  {
    strcpy(FileName,NewName);
    *FileNameW=0;
  }
  return(Success);
}


void File::Write(const void *Data,int Size)
{
    //dummy_printf("Writing:%s",this->FileName);
  if (Size==0)
    return;
#ifndef _WIN_CE
  if (HandleType!=FILE_HANDLENORMAL)
    switch(HandleType)
    {
      case FILE_HANDLESTD:
#ifdef _WIN_32
        hFile=GetStdHandle(STD_OUTPUT_HANDLE);
#else
        hFile=sceKernelStdout();
#endif
        break;
      case FILE_HANDLEERR:
#ifdef _WIN_32
        hFile=GetStdHandle(STD_ERROR_HANDLE);
#else
        hFile=sceKernelStderr();
#endif
        break;
    }
#endif
  while (1)
  {
    bool Success;
#ifdef _WIN_32
    DWORD Written;
    if (HandleType!=FILE_HANDLENORMAL)
    {
      const int MaxSize=0x4000;
      for (int I=0;I<Size;I+=MaxSize)
        if (!(Success=WriteFile(hFile,(byte *)Data+I,Min(Size-I,MaxSize),&Written,NULL)))
          break;
    }
    else
      Success=WriteFile(hFile,Data,Size,&Written,NULL);
#else
    //dummy_printf("DoWrite");
    u8* data2;data2=(u8*)memalign(64,Size);
    memcpy(data2,Data,Size);
    int Written=sceIoWrite(hFile,data2,Size);//fwrite(Data,1,Size,hFile);
    //CODE BRUTALLY REMOVED
    //dummy_printf("%i",Written);
    free(data2);
    if(Written==Size){}else{dummy_printf("hFileWERROR!%s!",FileName);}
    Success=Written==Size;// && !0;ferror(hFile);
#endif
    if (!Success && AllowExceptions && HandleType==FILE_HANDLENORMAL)
    {
#if defined(_WIN_32) && !defined(SFX_MODULE) && !defined(RARDLL)
      int ErrCode=GetLastError();
      Int64 FilePos=Tell();
      Int64 FreeSize=GetFreeDisk(FileName);
      SetLastError(ErrCode);
      if (FreeSize>Size && FilePos-Size<=0xffffffff && FilePos+Size>0xffffffff)
        ErrHandler.WriteErrorFAT(FileName);
#endif
      if (ErrHandler.AskRepeatWrite(FileName))
      {
#ifndef _WIN_32
          //CODE BRUTALLY REMOVED
        //clearerr(hFile);
#endif
        if (Written<Size && Written>0)
          Seek(Tell()-Written,SEEK_SET);
        continue;
      }
      ErrHandler.WriteError(NULL,FileName);
    }
    break;
  }
  LastWrite=true;
}


int File::Read(void *Data,int Size)
{
    //dummy_printf("Read:%s",this->FileName);
  Int64 FilePos;
  if (IgnoreReadErrors)
    FilePos=Tell();
  int ReadSize;
  while (true)
  {
    ReadSize=DirectRead(Data,Size);
    if (ReadSize==-1)
    {
      ErrorType=FILE_READERROR;
      if (AllowExceptions)
        if (IgnoreReadErrors)
        {
          ReadSize=0;
          for (int I=0;I<Size;I+=512)
          {
            Seek(FilePos+I,SEEK_SET);
            int SizeToRead=Min(Size-I,512);
            int ReadCode=DirectRead(Data,SizeToRead);
            ReadSize+=(ReadCode==-1) ? 512:ReadCode;
          }
        }
        else
        {
          if (HandleType==FILE_HANDLENORMAL && ErrHandler.AskRepeatRead(FileName))
            continue;
          ErrHandler.ReadError(FileName);
        }
    }
    break;
  }
  return(ReadSize);
}


int File::DirectRead(void *Data,int Size)
{
    //dummy_printf("DR");
#ifdef _WIN_32
  const int MaxDeviceRead=20000;
#endif
#ifndef _WIN_CE
  if (HandleType==FILE_HANDLESTD)
  {
#ifdef _WIN_32
    if (Size>MaxDeviceRead)
      Size=MaxDeviceRead;
    hFile=GetStdHandle(STD_INPUT_HANDLE);
#else
    hFile=sceKernelStdin();
#endif
  }
#endif
#ifdef _WIN_32
  DWORD Read;
  if (!ReadFile(hFile,Data,Size,&Read,NULL))
  {
    if (IsDevice() && Size>MaxDeviceRead)
      return(DirectRead(Data,MaxDeviceRead));
    if (HandleType==FILE_HANDLESTD && GetLastError()==ERROR_BROKEN_PIPE)
      return(0);
    return(-1);
  }
  return(Read);
#else
  if (LastWrite)
  {
      //CODE BRUTALLY REMOVED
    //fflush(hFile);
      //dummy_printf("flushA");
    LastWrite=false;
  }
  if(hFile){}else{dummy_printf("hFileERROR!");}
  //clearerr(hFile);
  int ReadSize=sceIoRead(hFile,Data,Size);//fread(Data,1,Size,hFile);
  if(ReadSize!=Size){return(-1);}
  //if (ferror(hFile))
  //  return(-1);
  return(ReadSize);
#endif
}


void File::Seek(Int64 Offset,int Method)
{
  if (!RawSeek(Offset,Method) && AllowExceptions)
    ErrHandler.SeekError(FileName);
}


bool File::RawSeek(Int64 Offset,int Method)
{
  if (hFile==BAD_HANDLE)
    return(true);
  if (!is64plus(Offset) && Method!=SEEK_SET)
  {
    Offset=(Method==SEEK_CUR ? Tell():FileLength())+Offset;
    Method=SEEK_SET;
  }
#ifdef _WIN_32
  LONG HighDist=int64to32(Offset>>32);
  if (SetFilePointer(hFile,int64to32(Offset),&HighDist,Method)==0xffffffff &&
      GetLastError()!=NO_ERROR)
    return(false);
#else
  LastWrite=false;
#ifdef _LARGEFILE_SOURCE
  //if (fseeko(hFile,Offset,Method)!=0)
  if(sceIoLseek(hFile,(SceOff) Offset, Method)!=0)
#else
  //if (fseek(hFile,int64to32(Offset),Method)!=0)
  //dummy_printf("Seek:
    if(sceIoLseek(hFile,(SceOff)int64to32(Offset),Method)<0)
#endif
    return(false);
#endif
  return(true);
}


Int64 File::Tell()
{
#ifdef _WIN_32
  LONG HighDist=0;
  uint LowDist=SetFilePointer(hFile,0,&HighDist,FILE_CURRENT);
  if (LowDist==0xffffffff && GetLastError()!=NO_ERROR)
    if (AllowExceptions)
      ErrHandler.SeekError(FileName);
    else
      return(-1);
  return(int32to64(HighDist,LowDist));
#else
#ifdef _LARGEFILE_SOURCE
  //return(ftello(hFile));
  return sceIoLseek(hFile,(SceOff)0,SEEK_CUR);
#else
  //return(ftell(hFile));
  //CODE BRUTALLY REMOVED
  return sceIoLseek((SceUID)hFile,0,SEEK_CUR);
#endif
#endif
}


void File::Prealloc(Int64 Size)
{
#ifdef _WIN_32
  if (RawSeek(Size,SEEK_SET))
  {
    Truncate();
    Seek(0,SEEK_SET);
  }
#endif
}


byte File::GetByte()
{
  byte Byte=0;
  Read(&Byte,1);
  return(Byte);
}


void File::PutByte(byte Byte)
{
  Write(&Byte,1);
}


bool File::Truncate()
{
#ifdef _WIN_32
  return(SetEndOfFile(hFile));
#else
  return(false);
#endif
}


void File::SetOpenFileTime(RarTime *ftm,RarTime *ftc,RarTime *fta)
{
#ifdef _WIN_32
  bool sm=ftm!=NULL && ftm->IsSet();
  bool sc=ftc!=NULL && ftc->IsSet();
  bool sa=fta!=NULL && fta->IsSet();
  FILETIME fm,fc,fa;
  if (sm)
    ftm->GetWin32(&fm);
  if (sc)
    ftc->GetWin32(&fc);
  if (sa)
    fta->GetWin32(&fa);
  SetFileTime(hFile,sc ? &fc:NULL,sa ? &fa:NULL,sm ? &fm:NULL);
#endif
}


void File::SetCloseFileTime(RarTime *ftm,RarTime *fta)
{
#if defined(_UNIX) || defined(_EMX)
  SetCloseFileTimeByName(FileName,ftm,fta);
#endif
}


void File::SetCloseFileTimeByName(const char *Name,RarTime *ftm,RarTime *fta)
{
    /*
#if defined(_UNIX) || defined(_EMX)
  bool setm=ftm!=NULL && ftm->IsSet();
  bool seta=fta!=NULL && fta->IsSet();
  if (setm || seta)
  {
    struct utimbuf ut;
    if (setm)
      ut.modtime=ftm->GetUnix();
    else
      ut.modtime=fta->GetUnix();
    if (seta)
      ut.actime=fta->GetUnix();
    else
      ut.actime=ut.modtime;
    utime(Name,&ut);
  }
#endif
  */
}


void File::GetOpenFileTime(RarTime *ft)
{
#ifdef _WIN_32
  FILETIME FileTime;
  GetFileTime(hFile,NULL,NULL,&FileTime);
  *ft=FileTime;
#endif
#if defined(_UNIX) || defined(_EMX)
  //CODE BRUTALLY REMOVED
  //struct stat st;
  //fstat(fileno(hFile),&st);
  *ft=3;//st.st_mtime;
#endif
}


void File::SetOpenFileStat(RarTime *ftm,RarTime *ftc,RarTime *fta)
{
#ifdef _WIN_32
  SetOpenFileTime(ftm,ftc,fta);
#endif
}


void File::SetCloseFileStat(RarTime *ftm,RarTime *fta,uint FileAttr)
{
#ifdef _WIN_32
  SetFileAttr(FileName,FileNameW,FileAttr);
#endif
#ifdef _EMX
  SetCloseFileTime(ftm,fta);
  SetFileAttr(FileName,FileNameW,FileAttr);
#endif
#ifdef _UNIX
  SetCloseFileTime(ftm,fta);
  /*CODE BRUTALLY CHANGED*/
  //chmod(FileName,(mode_t)FileAttr);
#endif
}


Int64 File::FileLength()
{
  SaveFilePos SavePos(*this);
  Seek(0,SEEK_END);
  return(Tell());
}


void File::SetHandleType(FILE_HANDLETYPE Type)
{
  HandleType=Type;
}


bool File::IsDevice()
{
  if (hFile==BAD_HANDLE)
    return(false);
#ifdef _WIN_32
  uint Type=GetFileType(hFile);
  return(Type==FILE_TYPE_CHAR || Type==FILE_TYPE_PIPE);
#else
  //dummy_printf("IsDevice");
  //CODE BRUTALLY REMOVED
  //return(isatty(fileno(hFile)));
  return 0;
#endif
}


#ifndef SFX_MODULE
void File::fprintf(const char *fmt,...)
{
  va_list argptr;
  va_start(argptr,fmt);
  safebuf char Msg[2*NM+1024],OutMsg[2*NM+1024];
  vsprintf(Msg,fmt,argptr);
#ifdef _WIN_32
  for (int Src=0,Dest=0;;Src++)
  {
    char CurChar=Msg[Src];
    if (CurChar=='\n')
      OutMsg[Dest++]='\r';
    OutMsg[Dest++]=CurChar;
    if (CurChar==0)
      break;
  }
#else
  strcpy(OutMsg,Msg);
#endif
  Write(OutMsg,strlen(OutMsg));
  va_end(argptr);
}
#endif


bool File::RemoveCreated()
{
  RemoveCreatedActive++;
  bool RetCode=true;
  for (int I=0;I<sizeof(CreatedFiles)/sizeof(CreatedFiles[0]);I++)
    if (CreatedFiles[I]!=NULL)
    {
      CreatedFiles[I]->SetExceptions(false);
      bool Success;
      if (CreatedFiles[I]->NewFile)
        Success=CreatedFiles[I]->Delete();
      else
        Success=CreatedFiles[I]->Close();
      if (Success)
        CreatedFiles[I]=NULL;
      else
        RetCode=false;
    }
  RemoveCreatedActive--;
  return(RetCode);
}


#ifndef SFX_MODULE
long File::Copy(File &Dest,Int64 Length)
{
  Array<char> Buffer(0x10000);
  long CopySize=0;
  bool CopyAll=(Length==INT64ERR);

  while (CopyAll || Length>0)
  {
    Wait();
    int SizeToRead=(!CopyAll && Length<Buffer.Size()) ? int64to32(Length):Buffer.Size();
    int ReadSize=Read(&Buffer[0],SizeToRead);
    if (ReadSize==0)
      break;
    Dest.Write(&Buffer[0],ReadSize);
    CopySize+=ReadSize;
    if (!CopyAll)
      Length-=ReadSize;
  }
  return(CopySize);
}
#endif
