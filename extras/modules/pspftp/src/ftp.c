#include "std.h"
#include "ftp.h"
#include "sutils.h"
#include "psp_cfg.h"

#include <pspiofilemgr.h>
#include <pspiofilemgr_fcntl.h>
#include <pspiofilemgr_stat.h>
#include <pspiofilemgr_dirent.h>
#include "psp_init.h"

static u8 loc_ms_buf[TRANSFER_MS_BUFFER_SIZE];

int
mftpCreateDirIfNeeded(MftpConnection *con, char *Filename) 
{
    SceIoStat fileStats;

  char  DirPath[MAX_PATH_LENGTH];
  char *ScanDir;
  char *EndDir;

  strcpy(DirPath, Filename);
  EndDir  = strrchr(DirPath, '/');
  if (EndDir == (char *)0) return 0;
  EndDir[1] = '\0';

  /* Skip first slash */
  ScanDir  = DirPath;
  ScanDir  = strchr(ScanDir,'/');
  if (ScanDir == (char *)0) return 0;
  ScanDir = ScanDir + 1;
  
  while ( (EndDir = strchr(ScanDir,'/')) != (char *)0 ) {
    *EndDir = '\0';
    
        if (sceIoGetstat(DirPath, &fileStats) >= 0) {
          if (!FIO_SO_ISDIR(fileStats.st_attr)) {
        return -1;
      }

    } else {
      sceIoMkdir(DirPath, 0);
      sceKernelDelayThread(100000); 
    }
    *EndDir = '/';
    ScanDir = EndDir + 1;
  }
  return 0;
}

void 
sendResponse(MftpConnection *con, char* s) 
{
    strcat(con->sockCommandBuffer, s);
    if (endsWith(con->sockCommandBuffer, (char*)"\n")) {
        sceNetInetSend(con->sockCommand, con->sockCommandBuffer, strlen(con->sockCommandBuffer) , 0);
        strcpy(con->sockCommandBuffer, (char*)"");
    }
}

void sendResponseLn(MftpConnection *con, char* s) 
{
    strcat(con->sockCommandBuffer, s);
    strcat(con->sockCommandBuffer, (char*)"\r\n");
    sceNetInetSend(con->sockCommand, con->sockCommandBuffer, strlen(con->sockCommandBuffer) , 0);
    strcpy(con->sockCommandBuffer, (char*)"");
}

void sendData(MftpConnection *con, char* s) 
{
    strcat(con->sockDataBuffer, s);
    if (endsWith(con->sockDataBuffer, (char*)"\n")) {
        sceNetInetSend(con->sockData, con->sockDataBuffer, strlen(con->sockDataBuffer) , 0);
        strcpy(con->sockDataBuffer, (char*)"");
    }
}

void 
sendDataLn(MftpConnection *con, char* s) {
    strcat(con->sockDataBuffer, s);
    strcat(con->sockDataBuffer, (char*)"\r\n");
    sceNetInetSend(con->sockData, con->sockDataBuffer, strlen(con->sockDataBuffer) , 0);
    strcpy(con->sockDataBuffer, (char*)"");
}

unsigned short pasvPort=59735;
int 
openDataConnectionPASV(MftpConnection *con) 
{
    con->usePassiveMode=1;

    int err;

    struct sockaddr_in addrPort;
    memset(&addrPort, 0, sizeof(struct sockaddr_in));

    addrPort.sin_size = sizeof(struct sockaddr_in);
    addrPort.sin_family = AF_INET;
    addrPort.sin_port = htons(pasvPort);
    addrPort.sin_addr[0] = 0;
    addrPort.sin_addr[1] = 0;
    addrPort.sin_addr[2] = 0;
    addrPort.sin_addr[3] = 0;

    con->sockPASV = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
    if (con->sockPASV & 0x80000000) return 0;

    err = sceNetInetBind(con->sockPASV, &addrPort, sizeof(addrPort));
    if (err) return 0;

    err = sceNetInetListen(con->sockPASV, 1);
    if (err) return 0;

    pasvPort++;
    return 0;
}


int openDataConnection(MftpConnection *con) {
    int err;

    if (con->usePassiveMode) {
        struct sockaddr_in addrAccept;
        u32 cbAddrAccept;

        cbAddrAccept = sizeof(addrAccept);
        con->sockData = sceNetInetAccept(con->sockPASV, &addrAccept, (int*)&cbAddrAccept);
        if (con->sockData & 0x80000000) return 0;
    } else {
        struct sockaddr_in addrPort;
        memset(&addrPort, 0, sizeof(struct sockaddr_in));

        addrPort.sin_size = sizeof(struct sockaddr_in);
        addrPort.sin_family = AF_INET;
        addrPort.sin_port = htons(con->port_port);
        addrPort.sin_addr[0] = con->port_addr[0];
        addrPort.sin_addr[1] = con->port_addr[1];
        addrPort.sin_addr[2] = con->port_addr[2];
        addrPort.sin_addr[3] = con->port_addr[3];

        con->sockData = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
        if (con->sockData & 0x80000000) return 0;

        err = sceNetInetConnect(con->sockData, &addrPort, sizeof(struct sockaddr_in));

        if (err) return 0;
    }

    return 1;
}

int 
closeDataConnection(MftpConnection *con) {
    int err = 0;

    err |= sceNetInetClose(con->sockData);
    if (con->usePassiveMode) {
        err |= sceNetInetClose(con->sockPASV);
    }

    if (err) return 0; else return 1;
}

int mftpServerHello(MftpConnection *con) {
    sendResponseLn(con, (char*)"220 FTP Server Ready");
    return 0;
}

int mftpCommandPWD(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con,command)) {
        sendResponse(con, (char*)"257 \"");
        sendResponse(con, con->curDir);
        sendResponseLn(con, (char*)"\" is current directory.");
    }

    return 0;
}

int mftpCommandCWD(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con,command)) {
        char* newDir=skipWS(&command[3]);
        trimEndingWS(newDir);

        char parsedDir[MAX_PATH_LENGTH+1];

        char* pParsedDir=parsedDir;
        parsedDir[0]=0;


        char* parser=newDir;
        if ((*newDir)=='/') {
            strcpy(con->curDir, (char*)"/");
            parser++;
        }

        do {
            if ((*parser)==0 || (*parser)=='/') {
                *pParsedDir=0;
                if (strcmp(parsedDir, (char*)".")==0) {

                } else if (strcmp(parsedDir, (char*)"..")==0) {
                    char* pUp=con->curDir+strlen(con->curDir)-2;
                    while ( pUp>=con->curDir && (*pUp)!='/' ) {
                        pUp--;
                    }
                    if ((++pUp)>=con->curDir) {
                        *pUp=0;
                    }
                    if (con->curDir[0]==0) {
                        break;
                    }

                } else {
                    strcat(con->curDir, parsedDir);
              if (!endsWith(con->curDir, (char*)"/")) {
                  strcat(con->curDir, (char*)"/");
              }
                }
                pParsedDir=parsedDir;
            } else {
                (*pParsedDir++)=(*parser);
            }

        } while (*(parser++)!=0);
        
        if (!endsWith(con->curDir, (char*)"/")) {
            strcat(con->curDir, (char*)"/");
        }

        sendResponseLn(con, (char*)"250 CWD command successful.");
    }

    return 0;
}

int mftpCommandLIST(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con,command)) {
        if (openDataConnection(con)==0) {
            sendResponseLn(con, (char*)"425 impossible to open data connection.");
        } else {
            sendResponseLn(con, (char*)"150 Opening ASCII mode data connection for file list");

            char path[MAX_PATH_LENGTH+1];
            strcpy(path, con->root);
            strcat(path, con->curDir);

            int ret,fd;
            SceIoDirent curFile;

            fd = sceIoDopen(path);
            if (fd>0) {
                do {
                    memset(&curFile, 0, sizeof(SceIoDirent));

                    ret = sceIoDread(fd, &curFile);
                    
                    char sInt[16]; strcpy(sInt, (char*)"");

                    if (ret>0) {
                        if (FIO_S_ISDIR(curFile.d_stat.st_mode)) {

                            sendData(con, (char*)"drwxrwxrwx   2 root     root     ");
                            itoa(sInt, curFile.d_stat.st_size);
                            sendData(con, sInt);
                            sendData(con, (char*)" Jan 01  1970 ");
                            sendDataLn(con, curFile.d_name);

                        } else if (FIO_S_ISLNK(curFile.d_stat.st_mode)) {
                            sendData(con, (char*)"lrwxrwxrwx   1 root     root     ");
                            itoa(sInt, curFile.d_stat.st_size);
                            sendData(con, sInt);
                            sendData(con, (char*)" Jan 01  1970 ");
                            sendData(con, curFile.d_name);
                            sendData(con, (char*)" -> ");
                            sendDataLn(con, (char*)"???");
                        } else {
                            sendData(con, (char*)"-rwxrwxrwx   1 root     root     ");
                            itoa(sInt, curFile.d_stat.st_size);
                            sendData(con, sInt);
                            sendData(con, (char*)" Jan 01  1970 ");
                            sendDataLn(con, curFile.d_name);
                        }
                    }
                } while (ret>0);

                sceIoDclose(fd);
            }
            sendResponseLn(con, (char*)"226 Transfer complete.");
            closeDataConnection(con);
        }
    }

    return 0;
}

int mftpCommandNLST(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        if (openDataConnection(con)==0) {
            sendResponseLn(con, (char*)"425 impossible to open data connection.");
        } else {
            sendResponseLn(con, (char*)"150 Opening ASCII mode data connection for file list");

            char path[MAX_PATH_LENGTH+1];
            strcpy(path, con->root);
            strcat(path, con->curDir);

            int ret,fd;
            SceIoDirent curFile;

            fd = sceIoDopen(path);
            if (fd>0) {
                do {
                    memset(&curFile, 0, sizeof(SceIoDirent));

                    ret = sceIoDread(fd, &curFile);
                    
                    sendDataLn(con, curFile.d_name);
                } while (ret>0);

                sceIoDclose(fd);
            }

            sendResponseLn(con, (char*)"226 Transfer complete.");
            closeDataConnection(con);
        }
    }

    return 0;
}

int mftpCommandRETR(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        if (openDataConnection(con)==0) {
            sendResponseLn(con, (char*)"425 impossible to open data connection.");
        } else {
            char* fileName=skipWS(&command[5]);
            trimEndingWS(fileName);

            if (strlen(fileName)>0) {
                sendResponse(con, (char*)"150 Opening ASCII mode data connection for ");
                //TODO.txt (1805 bytes).
                sendResponse(con, fileName);
                sendResponseLn(con, (char*)".");

                char filePath[MAX_PATH_LENGTH];
                if (strStartsWith(fileName, (char*)"/")) {
                    strcpy(filePath, con->root);
                    strcat(filePath, fileName);
                } else {
                    strcpy(filePath, con->root);
                    strcat(filePath, con->curDir);
                    strcat(filePath, fileName);
                }

                SceIoStat fileStats;
                sceIoGetstat(filePath, &fileStats);

              if ((! FIO_SO_ISDIR(fileStats.st_attr)) &&
            (! FIO_SO_ISLNK(fileStats.st_attr))) {
                    int fdFile = sceIoOpen(filePath, PSP_O_RDONLY, 0777);
          int read_len = 0;
                    while (( read_len = sceIoRead(fdFile, loc_ms_buf, TRANSFER_MS_BUFFER_SIZE))>0) {
            u8* scan_buf = loc_ms_buf;
            int send_len = 0;
            while (read_len > 0) {
              if (read_len > TRANSFER_SEND_BUFFER_SIZE) {
                            sceNetInetSend(con->sockData, scan_buf, TRANSFER_SEND_BUFFER_SIZE, 0);
                scan_buf += TRANSFER_SEND_BUFFER_SIZE;
                read_len -= TRANSFER_SEND_BUFFER_SIZE;
              } else {
                            sceNetInetSend(con->sockData, scan_buf, read_len, 0);
                read_len = 0;
              }
            }
                    }
                    sceIoClose(fdFile);
                    sendResponseLn(con, (char*)"226 Transfer complete.");
                } else {
                    sendResponse(con, (char*)"550 ");
                    sendResponse(con, fileName);
                    sendResponseLn(con, (char*)": not a regular file.");
                }
            } else {
                sendResponseLn(con, (char*)"500 'RETR': command requires a parameter.");
            }

            
            closeDataConnection(con);
        }
    }

    return 0;
}

int mftpCommandSTOR(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        if (openDataConnection(con)==0) {
            sendResponseLn(con, (char*)"425 impossible to open data connection.");
        } else {            
            char* fileName=skipWS(&command[5]);
            trimEndingWS(fileName);

            if (strlen(fileName)>0) {
                sendResponse(con, (char*)"150 Opening ASCII mode data connection for ");
                //TODO.txt (1805 bytes).
                sendResponse(con, fileName);
                sendResponseLn(con, (char*)".");
                
                char filePath[MAX_PATH_LENGTH];
                if (strStartsWith(fileName, (char*)"/")) {
                    strcpy(filePath, con->root);
                    strcat(filePath, fileName);
                } else {
                    strcpy(filePath, con->root);
                    strcat(filePath, con->curDir);
                    strcat(filePath, fileName);
                }

                if (mftpCreateDirIfNeeded(con, filePath) < 0) {
                    sendResponseLn(con, (char*)"552 'STOR': Requested file action aborted.");
                } else {
                    int fdFile = sceIoOpen(filePath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
                    int rcv_max = TRANSFER_MS_BUFFER_SIZE - TRANSFER_RECV_BUFFER_SIZE;
                    int rcv_left= rcv_max;
                    int rcv_len = 0;
                    int rcv_sum = 0;
                    u8* scan_buf = loc_ms_buf;
                    while (( rcv_len = sceNetInetRecv(con->sockData, (u8*)scan_buf, TRANSFER_RECV_BUFFER_SIZE, 0))>0) {
                        rcv_sum  += rcv_len;
                        rcv_left -= rcv_len;
                        scan_buf += rcv_len;
                        if (rcv_left <= 0) {
                            sceIoWrite(fdFile, loc_ms_buf, rcv_sum ); 
                            rcv_sum = 0;
                            rcv_left= rcv_max;
                            scan_buf= loc_ms_buf;
                        }
                    }
                    if (rcv_sum) {
                        sceIoWrite(fdFile, loc_ms_buf, rcv_sum); 
                    }
                    sceIoClose(fdFile);
                    closeDataConnection(con);
                    sendResponseLn(con, (char*)"226 Transfer complete.");
                }
            } else {
                sendResponseLn(con, (char*)"500 'STOR': command requires a parameter.");
                closeDataConnection(con);
            }
        }
    }

    return 0;
}

int mftpCommandSIZE(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        char* fileName=skipWS(&command[5]);
        trimEndingWS(fileName);

        if (strlen(fileName)>0) {
            char filePath[MAX_PATH_LENGTH];
            if (strStartsWith(fileName, (char*)"/")) {
                strcpy(filePath, con->root);
                strcat(filePath, fileName);
            } else {
                strcpy(filePath, con->root);
                strcat(filePath, con->curDir);
                strcat(filePath, fileName);
            }

            SceIoStat fileStats;
            sceIoGetstat(filePath, &fileStats);

            if ((! FIO_SO_ISDIR(fileStats.st_attr)) &&
          (! FIO_SO_ISLNK(fileStats.st_attr))) {
                char tmp[32];
                itoa(tmp, fileStats.st_size);
                sendResponse(con, (char*)"213 ");
                sendResponseLn(con, tmp);
            } else {
                sendResponse(con, (char*)"550 ");
                sendResponse(con, fileName);
                sendResponseLn(con, (char*)": not a regular file.");
            }
            
        } else {
            sendResponseLn(con, (char*)"500 'SIZE': command requires a parameter.");
        }
    }

    return 0;
}

int mftpCommandDELE(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        char* fileName=skipWS(&command[5]);
        trimEndingWS(fileName);

        if (strlen(fileName)>0) {

            char filePath[MAX_PATH_LENGTH];
            if (strStartsWith(fileName, (char*)"/")) {
                strcpy(filePath, con->root);
                strcat(filePath, fileName);
            } else {
                strcpy(filePath, con->root);
                strcat(filePath, con->curDir);
                strcat(filePath, fileName);
            }

            if(fileExists(filePath)){
                sceIoRemove(filePath);
            }
            else {
                //not a file? must be a folder
                strcat(filePath, "/");
                recursiveFolderDelete(filePath); //try to delete folder content
            }

            sendResponseLn(con, (char*)"250 DELE command successful.");
        } else {
            sendResponseLn(con, (char*)"500 'DELE': command requires a parameter.");
        }
    }

    return 0;
}

int mftpCommandRMD(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        char* fileName=skipWS(&command[4]);
        trimEndingWS(fileName);

        if (strlen(fileName)>0) {

            char filePath[MAX_PATH_LENGTH];
            if (strStartsWith(fileName, (char*)"/")) {
                strcpy(filePath, con->root);
                strcat(filePath, fileName);
            } else {
                strcpy(filePath, con->root);
                strcat(filePath, con->curDir);
                strcat(filePath, fileName);
            }

            trimEndingChar(filePath, '/');
            sceIoRmdir(filePath);
      sceKernelDelayThread(100000); 
            sendResponseLn(con, (char*)"250 RMD command successful.");
        } else {
            sendResponseLn(con, (char*)"500 'RMD': command requires a parameter.");
        }
    }

    return 0;
}

int mftpCommandMKD(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        char* fileName=skipWS(&command[4]);
        trimEndingWS(fileName);

        if (strlen(fileName)>0) {

            char filePath[MAX_PATH_LENGTH];
            if (strStartsWith(fileName, (char*)"/")) {
                strcpy(filePath, con->root);
                strcat(filePath, fileName);
            } else {
                strcpy(filePath, con->root);
                strcat(filePath, con->curDir);
                strcat(filePath, fileName);
            }

            trimEndingChar(filePath, '/');

      if (mftpCreateDirIfNeeded(con, filePath) < 0) {
              sendResponseLn(con, (char*)"552 'MKD': Requested file action aborted.");
      } else {
              sceIoMkdir(filePath, 0);
        sceKernelDelayThread(100000); 
       
              sendResponse(con, (char*)"257 \"");
              sendResponse(con, fileName);
              sendResponseLn(con, (char*)"\" - Directory successfully created.");
      }
        } else {
            sendResponseLn(con, (char*)"500 'MKD': command requires a parameter.");
        }
    }

    return 0;
}

int mftpCommandHELP(MftpConnection *con, char* command) 
{
    sendResponseLn(con, (char*)"214-The following commands are recognized (* =>'s unimplemented).");
    sendResponseLn(con, (char*)"214-USER    PASS    ACCT*   CWD     XCWD*    CDUP    XCUP*    SMNT*");
    sendResponseLn(con, (char*)"214-QUIT    REIN*   PORT    PASV    TYPE    STRU*    MODE*    RETR");
    sendResponseLn(con, (char*)"214-STOR    STOU*   APPE*    ALLO*   REST*    RNFR     RNTO     ABOR*");
    sendResponseLn(con, (char*)"214-DELE    MDTM*    RMD     XRMD*    MKD     XMKD*    PWD     XPWD*");
    sendResponseLn(con, (char*)"214-SIZE    LIST    NLST    SITE    SYST    STAT*    HELP    NOOP");
    sendResponseLn(con, (char*)"214 Direct comments to psp@amoks.com.");

    return 0;
}

int mftpCommandSITE(MftpConnection *con, char * command) 
{
    char* param=skipWS(&command[5]);
    trimEndingWS(param);
    toUpperCase(param);
    if (strcmp(param, (char*)"HELP")==0) {
        sendResponseLn(con, (char*)"214-The following SITE commands are recognized (* =>'s unimplemented).");
        sendResponseLn(con, (char*)"214-HELP");
        sendResponseLn(con, (char*)"214 Direct comments to psp@amoks.com.");
    } else if (strlen(param)==0) {
        sendResponseLn(con, (char*)"500 'SITE' requires argument.");
    } else {
        sendResponse(con, (char*)"500 '");
        sendResponse(con, command);
        sendResponseLn(con, (char*)"' not understood.");
    }

    return 0;
}

int mftpCommandPORT(MftpConnection *con, char* command) 
{
    int params[6];
    char decimByte[4];
    char* pDecimByte=decimByte;
    char* pParams=skipWS(&command[5]);

    int state=0; int err=0; int nbParams=0;
    do {
        if (state==0 && *pParams>='0' && *pParams<='9') {
            state=1;
            pParams--;
        } else if (state==1 && *pParams>='0' && *pParams<='9') {
            if (pDecimByte-decimByte<=2) {
                *(pDecimByte++)=*pParams;
            } else {
                err=1;
            }
        } else if (state==1 && (*pParams==',' || *pParams==0) && nbParams<6) {
            *pDecimByte=0;

            if (strlen(decimByte)==0) {
                err=1;
            } else {
                int param=0;
                char* tmp=decimByte+strlen(decimByte)-1;
                int pow=1;
                while (tmp>=decimByte && err==0) {
                    
                    if (*tmp>='0' && *tmp<='9') {
                        param+= ((*tmp)-48)*pow;
                        pow=pow*10;
                    } else {
                        err=1;
                    }

                    tmp--;
                }

                if (err==0) {
                    params[nbParams++]=param;
                    pDecimByte=decimByte;
                }
            }
        } else {
            err=1;
        }

    } while (*(pParams++)!=0 && err==0);

    if (err==0 && state==1 && nbParams==6) {
        con->usePassiveMode=0;

        con->port_addr[0]=(unsigned char) params[0];
        con->port_addr[1]=(unsigned char) params[1];
        con->port_addr[2]=(unsigned char) params[2];
        con->port_addr[3]=(unsigned char) params[3];
        con->port_port=((unsigned char) params[4]<<8) | ((unsigned char) params[5]);
        sendResponseLn(con, (char*)"200 PORT command successful.");
    } else {
        con->port_addr[0]=0;
        con->port_addr[1]=0;
        con->port_addr[2]=0;
        con->port_addr[3]=0;
        con->port_port=0;
        sendResponseLn(con, (char*)"500 illegal PORT command.");
    }

    return 0;
}

int mftpCommandUSER(MftpConnection *con, char* command) 
{
    if (con->userLoggedIn) {
        sendResponseLn(con, (char*)"503 You are already logged in!");
    } else {
        con->user[0]=0;
        con->pass[0]=0;
        char* pUser=skipWS(&command[5]);
        trimEndingWS(pUser);
        if (strlen(pUser)==0) {
            sendResponseLn(con, (char*)"500 'USER': command requires a parameter.");
        } else {
            strncpy(con->user, pUser, MAX_USER_LENGTH);
            sendResponse(con, (char*)"331 Password required for ");
            sendResponse(con, con->user);
            sendResponseLn(con, (char*)".");
        }
    }

    return 0;
}

int mftpCommandPASS(MftpConnection *con, char* command) 
{
    if (con->userLoggedIn) {
        sendResponseLn(con, (char*)"503 You are already logged in!");
    } else {
        if (strlen(con->user)==0) {
            sendResponseLn(con, (char*)"503 Login with USER first.");
        } 
    else 
    {
            con->pass[0]=0;
            char* pPass=skipWS(&command[5]);
            trimEndingWS(pPass);
            if (strlen(pPass)==0) {
                sendResponseLn(con, (char*)"500 'PASS': command requires a parameter.");
            } else {
                strncpy(con->pass, pPass, MAX_PASS_LENGTH);
        if (mftp_config.auth_required) {
          mftpUser_t *CheckUser = cfg_get_user(con->user, con->pass);
          if (CheckUser == (mftpUser_t *)0) {
                      sendResponseLn(con, (char*)"530 You're not allowed to log in.");
                      con->userLoggedIn=0;
                      con->user[0]=0; con->pass[0]=0;
            return 0;
          } else {
            strcpy(con->root,CheckUser->root);
          }
        }
                sendResponseLn(con, (char*)"230 You're logged in.");
                con->userLoggedIn=1;
            }
        }
    }

    return 0;
}

int mftpCommandTYPE(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        char* pParam1=skipWS(&command[5]);
        trimEndingWS(pParam1);
        if (strlen(pParam1)==0) {
            sendResponseLn(con, (char*)"500 'TYPE': command requires a parameter.");
        } else if (strlen(pParam1)==1 && (*pParam1=='A' || *pParam1=='E' || *pParam1=='I' || *pParam1=='L')) {
            con->transfertType=*pParam1;
            sendResponse(con, (char*)"200 Type set to ");
            sendResponse(con, pParam1);
            sendResponseLn(con, (char*)".");
        } else {
            sendResponseLn(con, (char*)"500 'TYPE': 2 parameters extended version not understood.");
        }
    }

    return 0;
}

int mftpCommandSYST(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        sendResponseLn(con, (char*)"215 UNIX Type: L8");
    }

    return 0;
}

int mftpCommandPASV(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        
        char tmp[32];
        strncpy(tmp, con->serverIp, 31);
        strReplaceChar(tmp, '.', ',');

        sendResponse(con, (char*)"227 Entering Passive Mode (");
        sendResponse(con, tmp);
        sendResponse(con, (char*)",");
        itoa(tmp, (pasvPort>>8) & 0xFF);
        sendResponse(con, tmp);
        sendResponse(con, (char*)",");
        itoa(tmp, pasvPort & 0xFF);
        sendResponse(con, tmp);
        sendResponseLn(con, (char*)").");

        openDataConnectionPASV(con);
    }

    return 0;
}

int mftpCommandNOOP(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        sendResponseLn(con, (char*)"200 NOOP command successful.");
    }

    return 0;
}

int mftpCommandQUIT(MftpConnection *con, char* command) 
{
    closeDataConnection(con);

    return -1;
}

int mftpCommandRNFR(MftpConnection *con, char* command) 
{
    if (mftpRestrictedCommand(con, command)) {
        char* fileName=skipWS(&command[4]);
        trimEndingWS(fileName);

        if (strlen(fileName)>0) {

            char filePath[MAX_PATH_LENGTH];
            if (strStartsWith(fileName, "/")) {
                strcpy(filePath, con->root);
                strcat(filePath, fileName);
            } else {
                strcpy(filePath, con->root);
                strcat(filePath, con->curDir);
                strcat(filePath, fileName);
            }
 
      strncpy(con->renameFromFileName,filePath,MAX_PATH_LENGTH);

          sendResponseLn(con, (char*)"350 File or directory exists, ready for destination name.");
      con->renameFrom = 1;
        } else {
            sendResponseLn(con, (char*)"500 'RNFR': command requires a parameter.");
      con->renameFrom = 0;
        }
    }

  return 0;
}

int mftpCommandRNTO(MftpConnection *con, char* command) 
{
  if (! con->renameFrom) {
      sendResponseLn(con, (char*)"503 'RNTO': Bad sequence of commands.");
    return 0;
  }
  con->renameFrom = 0;

    if (mftpRestrictedCommand(con, command)) {
        char* fileName=skipWS(&command[4]);
        trimEndingWS(fileName);

    if (strlen(fileName)>0) {

            char filePath[MAX_PATH_LENGTH];
            if (strStartsWith(fileName, "/")) {
                strcpy(filePath, con->root);
                strcat(filePath, fileName);
            } else {
                strcpy(filePath, con->root);
                strcat(filePath, con->curDir);
                strcat(filePath, fileName);
            }

      sceIoRename(con->renameFromFileName,filePath);
          sendResponseLn(con, (char*)"250 Rename successfull.");
        } else {
            sendResponseLn(con, (char*)"500 'RNTO': command requires a parameter.");
        }
    }

  return 0;
}

int mftpRestrictedCommand(MftpConnection *con, char* command) 
{
    if ((mftp_config.auth_required) && (!con->userLoggedIn)) {
        sendResponseLn(con, (char*)"530 Please login with USER and PASS.");
        return 0;
    } 
  return 1;
}

int mftpDispatch(MftpConnection *con, char* command) 
{
    char uCommand[MAX_COMMAND_LENGTH+1];
    strncpy(uCommand, command, MAX_COMMAND_LENGTH);
    toUpperCase(uCommand);
  int renameFrom = 0;

    int ret=0;
    if (strlen(uCommand)>0) {
        if (strcmp(uCommand, (char*)"PWD")==0) {
            ret=mftpCommandPWD(con, command);
        } else if (strcmp(uCommand, (char*)"NLST")==0) {
            ret=mftpCommandNLST(con, command);
        } else if (strcmp(uCommand, (char*)"LIST")==0 || strStartsWith(uCommand, (char*)"LIST ")) {
            ret=mftpCommandLIST(con, command);
        } else if (strStartsWith(uCommand, (char*)"RETR ")) {
            ret=mftpCommandRETR(con, command);
        } else if (strStartsWith(uCommand, (char*)"STOR ")) {
            ret=mftpCommandSTOR(con, command);
        } else if (strStartsWith(uCommand, (char*)"SIZE ")) {
            ret=mftpCommandSIZE(con, command);
        } else if (strStartsWith(uCommand, (char*)"DELE ")) {
            ret=mftpCommandDELE(con, command);
        } else if (strStartsWith(uCommand, (char*)"RMD ")) {
            ret=mftpCommandRMD(con, command);
        } else if (strStartsWith(uCommand, (char*)"MKD ")) {
            ret=mftpCommandMKD(con, command);
        } else if (strStartsWith(uCommand, (char*)"RNFR ")) {
            ret=mftpCommandRNFR(con, command);
      renameFrom = con->renameFrom;
        } else if (strStartsWith(uCommand, (char*)"RNTO ")) {
            ret=mftpCommandRNTO(con, command);
        } else if (strcmp(uCommand, (char*)"CDUP")==0) {
            ret=mftpCommandCWD(con, (char*)"CWD ..");
        } else if (strStartsWith(uCommand, (char*)"CWD ")) {
            ret=mftpCommandCWD(con, command);
        } else if (strcmp(uCommand, (char*)"HELP")==0 || strStartsWith(uCommand, (char*)"HELP ")) {
            ret=mftpCommandHELP(con, command);
        } else if (strcmp(uCommand, (char*)"SITE")==0 || strStartsWith(uCommand, (char*)"SITE ")) {
            ret=mftpCommandSITE(con, command);
        } else if (strStartsWith(uCommand, (char*)"PORT ")) {
            ret=mftpCommandPORT(con, command);
        } else if (strStartsWith(uCommand, (char*)"USER ")) {
            ret=mftpCommandUSER(con, command);
        } else if (strStartsWith(uCommand, (char*)"PASS ")) {
            ret=mftpCommandPASS(con, command);
        } else if (strStartsWith(uCommand, (char*)"TYPE ")) {
            ret=mftpCommandTYPE(con, command);
        } else if (strcmp(uCommand, (char*)"SYST")==0) {
            ret=mftpCommandSYST(con, command);
        } else if (strcmp(uCommand, (char*)"PASV")==0) {
            ret=mftpCommandPASV(con, command);
        } else if (strcmp(uCommand, (char*)"NOOP")==0) {
            ret=mftpCommandNOOP(con, command);
        } else if (strcmp(uCommand, (char*)"QUIT")==0) {
            ret=mftpCommandQUIT(con, command);
        } else {
            sendResponse(con, (char*)"500 ");
            sendResponse(con, command);
            sendResponseLn(con, (char*)" not understood.");
# ifdef DEBUG
      pspDebugPrintf("command not understood\n");
# endif
        }
    con->renameFrom = renameFrom;
    }

    return ret;
}
