#include "rar.hpp"

static void ListFileHeader(FileHeader &hd,bool Verbose,bool Technical,bool &TitleShown,bool Bare);
static void ListSymLink(Archive &Arc);
static void ListFileAttr(uint A,int HostOS);
static void ListOldSubHeader(Archive &Arc);
static void ListNewSubHeader(CommandData *Cmd,Archive &Arc,bool Technical);

void ListArchive(CommandData *Cmd)
{
  Int64 SumPackSize=0,SumUnpSize=0;
  uint ArcCount=0,SumFileCount=0;
  bool Technical=(Cmd->Command[1]=='T');
  bool Bare=(Cmd->Command[1]=='B');
  bool Verbose=(*Cmd->Command=='V');

  char ArcName[NM];
  wchar ArcNameW[NM];

  while (Cmd->GetArcName(ArcName,ArcNameW,sizeof(ArcName)))
  {
    Archive Arc(Cmd);
#ifdef _WIN_32
    Arc.RemoveSequentialFlag();
#endif
    if (!Arc.WOpen(ArcName,ArcNameW))
      continue;
    bool FileMatched=true;
    while (1)
    {
      Int64 TotalPackSize=0,TotalUnpSize=0;
      uint FileCount=0;
      if (Arc.IsArchive(true))
      {
        if (!Arc.IsOpened())
          break;
        bool TitleShown=false;
        if (!Bare)
        {
          Arc.ViewComment();
          dummy_printf("\n");
          if (Arc.Solid)
            dummy_printf(St(MListSolid));
          if (Arc.SFXSize>0)
            dummy_printf(St(MListSFX));
          if (Arc.Volume)
            if (Arc.Solid)
              dummy_printf(St(MListVol1));
            else
              dummy_printf(St(MListVol2));
          else
            if (Arc.Solid)
              dummy_printf(St(MListArc1));
            else
              dummy_printf(St(MListArc2));
          dummy_printf(" %s\n",Arc.FileName);
          if (Technical)
          {
            if (Arc.Protected)
              dummy_printf(St(MListRecRec));
            if (Arc.Locked)
              dummy_printf(St(MListLock));
          }
        }
        while(Arc.ReadHeader()>0)
        {
          switch(Arc.GetHeaderType())
          {
            case FILE_HEAD:
              IntToExt(Arc.NewLhd.FileName,Arc.NewLhd.FileName);
              if ((FileMatched=Cmd->IsProcessFile(Arc.NewLhd))==true)
              {
                ListFileHeader(Arc.NewLhd,Verbose,Technical,TitleShown,Bare);
                if (!(Arc.NewLhd.Flags & LHD_SPLIT_BEFORE))
                {
                  TotalUnpSize+=Arc.NewLhd.FullUnpSize;
                  FileCount++;
                }
                TotalPackSize+=Arc.NewLhd.FullPackSize;
                if (Technical)
                  ListSymLink(Arc);
#ifndef SFX_MODULE
                if (Verbose)
                  Arc.ViewFileComment();
#endif
              }
              break;
#ifndef SFX_MODULE
            case SUB_HEAD:
              if (Technical && FileMatched && !Bare)
                ListOldSubHeader(Arc);
              break;
#endif
            case NEWSUB_HEAD:
              if (FileMatched && !Bare)
              {
                if (Technical)
                  ListFileHeader(Arc.SubHead,Verbose,true,TitleShown,false);
                ListNewSubHeader(Cmd,Arc,Technical);
              }
              break;
          }
          Arc.SeekToNext();
        }
        if (!Bare)
          if (TitleShown)
          {
            dummy_printf("\n");
            for (int I=0;I<79;I++)
              dummy_printf("-");
            char UnpSizeText[20];
            itoa(TotalUnpSize,UnpSizeText);
        
            char PackSizeText[20];
            itoa(TotalPackSize,PackSizeText);
        
            dummy_printf("\n%5lu %16s %8s %3d%%",FileCount,UnpSizeText,
                    PackSizeText,ToPercent(TotalPackSize,TotalUnpSize));
            SumFileCount+=FileCount;
            SumUnpSize+=TotalUnpSize;
            SumPackSize+=TotalPackSize;
#ifndef SFX_MODULE
            if (Arc.EndArcHead.Flags & EARC_VOLNUMBER)
            {
              dummy_printf("       ");
              dummy_printf(St(MVolumeNumber),Arc.EndArcHead.VolNumber+1);
            }
#endif
            dummy_printf("\n");
          }
          else
            dummy_printf(St(MListNoFiles));

        ArcCount++;

#ifndef NOVOLUME
        if (Cmd->VolSize!=0 && ((Arc.NewLhd.Flags & LHD_SPLIT_AFTER) ||
            Arc.GetHeaderType()==ENDARC_HEAD &&
            (Arc.EndArcHead.Flags & EARC_NEXT_VOLUME)!=0) &&
            MergeArchive(Arc,NULL,false,*Cmd->Command))
        {
          Arc.Seek(0,SEEK_SET);
        }
        else
#endif
          break;
      }
      else
      {
        if (Cmd->ArcNames->ItemsCount()<2 && !Bare)
          dummy_printf(St(MNotRAR),Arc.FileName);
        break;
      }
    }
  }
  if (ArcCount>1 && !Bare)
  {
    char UnpSizeText[20],PackSizeText[20];
    itoa(SumUnpSize,UnpSizeText);
    itoa(SumPackSize,PackSizeText);
    dummy_printf("\n%5lu %16s %8s %3d%%\n",SumFileCount,UnpSizeText,
            PackSizeText,ToPercent(SumPackSize,SumUnpSize));
  }
}


void ListFileHeader(FileHeader &hd,bool Verbose,bool Technical,bool &TitleShown,bool Bare)
{
  if (!Bare)
  {
    if (!TitleShown)
    {
      if (Verbose)
        dummy_printf(St(MListPathComm));
      else
        dummy_printf(St(MListName));
      dummy_printf(St(MListTitle));
      if (Technical)
        dummy_printf(St(MListTechTitle));
      for (int I=0;I<79;I++)
        dummy_printf("-");
      TitleShown=true;
    }

    if (hd.HeadType==NEWSUB_HEAD)
      dummy_printf(St(MSubHeadType),hd.FileName);

    dummy_printf("\n%c",(hd.Flags & LHD_PASSWORD) ? '*' : ' ');
  }

  char *Name=hd.FileName;

#ifdef UNICODE_SUPPORTED
  char ConvertedName[NM];
  if ((hd.Flags & LHD_UNICODE)!=0 && *hd.FileNameW!=0 && UnicodeEnabled())
  {
    if (WideToChar(hd.FileNameW,ConvertedName) && *ConvertedName!=0)
      Name=ConvertedName;
  }
#endif

  if (Bare)
  {
    dummy_printf("%s\n",Verbose ? Name:PointToName(Name));
    return;
  }

  if (Verbose)
    dummy_printf("%s\n%12s ",Name,"");
  else
    dummy_printf("%-12s",PointToName(Name));

  char UnpSizeText[20],PackSizeText[20];
  if (hd.FullUnpSize==INT64MAX)
    strcpy(UnpSizeText,"?");
  else
    itoa(hd.FullUnpSize,UnpSizeText);
  itoa(hd.FullPackSize,PackSizeText);

  dummy_printf(" %8s %8s ",UnpSizeText,PackSizeText);

  if ((hd.Flags & LHD_SPLIT_BEFORE) && (hd.Flags & LHD_SPLIT_AFTER))
    dummy_printf(" <->");
  else
    if (hd.Flags & LHD_SPLIT_BEFORE)
      dummy_printf(" <--");
    else
      if (hd.Flags & LHD_SPLIT_AFTER)
        dummy_printf(" -->");
      else
        dummy_printf("%3d%%",ToPercent(hd.FullPackSize,hd.FullUnpSize));

  char DateStr[50];
  hd.mtime.GetText(DateStr,false);
  dummy_printf(" %s ",DateStr);

  if (hd.HeadType==NEWSUB_HEAD)
    dummy_printf("  %c....B  ",(hd.SubFlags & SUBHEAD_FLAGS_INHERITED) ? 'I' : '.');
  else
    ListFileAttr(hd.FileAttr,hd.HostOS);

  dummy_printf(" %8.8X",hd.FileCRC);
  dummy_printf(" m%d",hd.Method-0x30);
  if ((hd.Flags & LHD_WINDOWMASK)<=6*32)
    dummy_printf("%c",((hd.Flags&LHD_WINDOWMASK)>>5)+'a');
  else
    dummy_printf(" ");
  dummy_printf(" %d.%d",hd.UnpVer/10,hd.UnpVer%10);

  static const char *RarOS[]={
    "DOS","OS/2","Win95/NT","Unix","MacOS","BeOS","WinCE","","",""
  };

  if (Technical)
    dummy_printf("\n%22s %8s %4s",
            (hd.HostOS<sizeof(RarOS)/sizeof(RarOS[0]) ? RarOS[hd.HostOS]:""),
            (hd.Flags & LHD_SOLID) ? St(MYes):St(MNo),
            (hd.Flags & LHD_VERSION) ? St(MYes):St(MNo));
}


void ListSymLink(Archive &Arc)
{
  if (Arc.NewLhd.HostOS==HOST_UNIX && (Arc.NewLhd.FileAttr & 0xF000)==0xA000)
  {
    char FileName[NM];
    int DataSize=Min(Arc.NewLhd.PackSize,sizeof(FileName)-1);
    Arc.Read(FileName,DataSize);
    FileName[DataSize]=0;
    dummy_printf("\n%22s %s","-->",FileName);
  }
}


void ListFileAttr(uint A,int HostOS)
{
  switch(HostOS)
  {
    case HOST_MSDOS:
    case HOST_OS2:
    case HOST_WIN32:
    case HOST_MACOS:
      dummy_printf(" %c%c%c%c%c%c%c  ",
              (A & 0x08) ? 'V' : '.',
              (A & 0x10) ? 'D' : '.',
              (A & 0x01) ? 'R' : '.',
              (A & 0x02) ? 'H' : '.',
              (A & 0x04) ? 'S' : '.',
              (A & 0x20) ? 'A' : '.',
              (A & 0x800) ? 'C' : '.');
      break;
    case HOST_UNIX:
    case HOST_BEOS:
      switch (A & 0xF000)
      {
        case 0x4000:
          dummy_printf("d");
          break;
        case 0xA000:
          dummy_printf("l");
          break;
        default:
          dummy_printf("-");
          break;
      }
      dummy_printf("%c%c%c%c%c%c%c%c%c",
              (A & 0x0100) ? 'r' : '-',
              (A & 0x0080) ? 'w' : '-',
              (A & 0x0040) ? ((A & 0x0800) ? 's':'x'):((A & 0x0800) ? 'S':'-'),
              (A & 0x0020) ? 'r' : '-',
              (A & 0x0010) ? 'w' : '-',
              (A & 0x0008) ? ((A & 0x0400) ? 's':'x'):((A & 0x0400) ? 'S':'-'),
              (A & 0x0004) ? 'r' : '-',
              (A & 0x0002) ? 'w' : '-',
              (A & 0x0001) ? 'x' : '-');
      break;
  }
}


#ifndef SFX_MODULE
void ListOldSubHeader(Archive &Arc)
{
  switch(Arc.SubBlockHead.SubType)
  {
    case EA_HEAD:
      dummy_printf(St(MListEAHead));
      break;
    case UO_HEAD:
      dummy_printf(St(MListUOHead),Arc.UOHead.OwnerName,Arc.UOHead.GroupName);
      break;
    case MAC_HEAD:
      dummy_printf(St(MListMACHead1),Arc.MACHead.fileType>>24,Arc.MACHead.fileType>>16,Arc.MACHead.fileType>>8,Arc.MACHead.fileType);
      dummy_printf(St(MListMACHead2),Arc.MACHead.fileCreator>>24,Arc.MACHead.fileCreator>>16,Arc.MACHead.fileCreator>>8,Arc.MACHead.fileCreator);
      break;
    case BEEA_HEAD:
      dummy_printf(St(MListBeEAHead));
      break;
    case NTACL_HEAD:
      dummy_printf(St(MListNTACLHead));
      break;
    case STREAM_HEAD:
      dummy_printf(St(MListStrmHead),Arc.StreamHead.StreamName);
      break;
    default:
      dummy_printf(St(MListUnkHead),Arc.SubBlockHead.SubType);
      break;
  }
}
#endif


void ListNewSubHeader(CommandData *Cmd,Archive &Arc,bool Technical)
{
  if (Technical && Arc.SubHead.CmpName(SUBHEAD_TYPE_CMT) &&
      (Arc.SubHead.Flags & LHD_SPLIT_BEFORE)==0 && !Cmd->DisableComment)
  {
    Array<byte> CmtData;
    int ReadSize=Arc.ReadCommentData(CmtData);
    if (ReadSize!=0)
    {
      dummy_printf(St(MFileComment));
      OutComment((char *)&CmtData[0],ReadSize);
    }
  }
  if (Arc.SubHead.CmpName(SUBHEAD_TYPE_STREAM) &&
      (Arc.SubHead.Flags & LHD_SPLIT_BEFORE)==0)
  {
    int DestSize=Arc.SubHead.SubData.Size()/2;
    wchar DestNameW[NM];
    char DestName[NM];
    if (DestSize<sizeof(DestName))
    {
      RawToWide(&Arc.SubHead.SubData[0],DestNameW,DestSize);
      DestNameW[DestSize]=0;
      WideToChar(DestNameW,DestName);
      dummy_printf("\n %s",DestName);
    }
  }
}
