#include "std.h"
#include "ftp.h"
#include "ftpd.h"
#include "psp_cfg.h"
#include "sutils.h"
#include "psp_init.h"

typedef struct thread_list {
    struct thread_list *next;
    int                 thread_id; 
} thread_list;

thread_list *mftp_thread_head = NULL;

SOCKET sockListen = 0;

static void (*msg_handler)(const char*) = NULL;
static char* ftpdevice = "ms0:";

void ftpdSetDevice(char* device){
  ftpdevice = device;
}

char* ftpdGetDevice(){
  return ftpdevice;
}

void ftpdSetMsgHandler(void (*handler)(const char*)){
    msg_handler = handler;
}

void mftpPrint(char *msg){
    if (msg_handler) msg_handler(msg);
}

static void
mftpAddThread(int thread_id) 
{
  thread_list *new_thread = (thread_list *)malloc(sizeof(thread_list));
  new_thread->next      = mftp_thread_head;
  new_thread->thread_id = thread_id;

  mftp_thread_head = new_thread;
}

static void
mftpDelThread(int thread_id) 
{
  thread_list **prev_thread = &mftp_thread_head;
  thread_list  *del_thread;

  del_thread = mftp_thread_head; 
  while (del_thread != (thread_list *)0) {
    if (del_thread->thread_id == thread_id) break;
    prev_thread = &del_thread->next;
    del_thread  = del_thread->next;
  }
  if (del_thread) {
    *prev_thread = del_thread->next;
    free(del_thread);
  }
}

int
ftpdExitHandler(SceSize argc, void *argv) 
{
  int err = 0;
  if (sockListen) {
      err = sceNetInetClose(sockListen);
  }

  thread_list  *scan_thread = mftp_thread_head; 
  while (scan_thread != (thread_list *)0) {
    sceKernelTerminateThread(scan_thread->thread_id);
    scan_thread = scan_thread->next;
  }
  return 0;
}

int 
mftpClientHandler(SceSize argc, void *argv) 
{
  int thid = sceKernelGetThreadId();
  mftpAddThread(thid);
    MftpConnection *con = *(MftpConnection **)argv;

    con->sockData =0;
    con->sockPASV =0;

  if (mftp_config.head_user) {
    strcpy(con->root,mftp_config.head_user->root);
  } else {
    const char* dev = ftpdGetDevice();
    con->root[0] = dev[0]; // m/e
    con->root[1] = dev[1]; // s/f
    con->root[2] = dev[2]; //  0
    con->root[3] = dev[3]; //  :
    con->root[4] = 0;
    if (strcmp(con->root, "ms0:") != 0 && strcmp(con->root, "ef0:") != 0){
        strcpy(con->root, "ms0:");
    }
  }

    memset(con->sockCommandBuffer, 0, 1024);
    memset(con->sockDataBuffer, 0, 1024);
    strcpy(con->curDir,"/");
    memset(con->user, 0, MAX_USER_LENGTH);
    memset(con->pass, 0, MAX_PASS_LENGTH);
  strcpy(con->renameFromFileName,"");
  con->renameFrom = 0;
    con->usePassiveMode=0;
    con->userLoggedIn=0;
    con->port_port=0;
    con->port_addr[0] = 0;
    con->port_addr[1] = 0;
    con->port_addr[2] = 0;
    con->port_addr[3] = 0;
    con->transfertType='A';

    int err;

    mftpServerHello(con);

    char messBuffer[1024];
    char readBuffer[1024];
    char lineBuffer[1024];
    int lineLen=0;
    int errLoop=0;
    while (errLoop>=0)
  {
  
      int nb = sceNetInetRecv(con->sockCommand, (u8*)readBuffer, 1024, 0);
      if (nb <= 0) break;

      int i=0; 
      while (i<nb) {
          if (readBuffer[i]!='\r') {
              lineBuffer[lineLen++]=readBuffer[i];
              if (readBuffer[i]=='\n' || lineLen==1024) {
                  lineBuffer[--lineLen]=0;
                  char* command=skipWS(lineBuffer);
                  trimEndingWS(command);

          sprintf(messBuffer, "> %s from %s", command, con->clientIp);
          mftpPrint(messBuffer);

                  if ((errLoop=mftpDispatch(con,command))<0) break;
                  lineLen=0;
              }
          }
          i++;
      }
  }

    err = sceNetInetClose(con->sockCommand);
    free(con);

  mftpDelThread(thid);
    sceKernelExitDeleteThread(0);
    
    return 0;
}

int 
ftpdLoop(SceSize argc, void *argv)
{
  char buffer_2[64];
  u32 err;
  SOCKET sockClient;

    struct sockaddr_in addrListen;
    struct sockaddr_in addrAccept;
    u32 cbAddrAccept;
    sockListen = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
    if (sockListen & 0x80000000) goto done;
    addrListen.sin_family = AF_INET;
    addrListen.sin_port = htons(21);
    addrListen.sin_addr[0] = 0;
    addrListen.sin_addr[1] = 0;
    addrListen.sin_addr[2] = 0;
    addrListen.sin_addr[3] = 0;

    // any
    err = sceNetInetBind(sockListen, &addrListen, sizeof(addrListen));
    if (err) goto done;
    err = sceNetInetListen(sockListen, 1);
    if (err) goto done;


  mftpPrint("Waiting for FTP clients");

  if (mftp_config.auth_required) {

    mftpPrint("User authentication required");

  } else {

    mftpPrint("Anonymous connection mode");

  }

  

  while (1) {
  
      cbAddrAccept = sizeof(addrAccept);

      sockClient = sceNetInetAccept(sockListen, &addrAccept, (int*)&cbAddrAccept);
      if (sockClient & 0x80000000) goto done;

    MftpConnection* con=(MftpConnection*)malloc(sizeof(MftpConnection));
    if (sceNetApctlGetInfo(8, (union SceNetApctlInfo*)con->serverIp) != 0) {
      goto done;
    }

    sprintf(con->clientIp, "%d.%d.%d.%d",
            addrAccept.sin_addr[0], addrAccept.sin_addr[1],
            addrAccept.sin_addr[2], addrAccept.sin_addr[3]);
    sprintf(buffer_2, "Connection from %s", con->clientIp);
    mftpPrint(buffer_2);

    con->sockCommand = sockClient;
      int client_id = sceKernelCreateThread("ftpd_client_loop", mftpClientHandler, 0x18, 0x10000, PSP_THREAD_ATTR_USBWLAN, 0);
      if(client_id >= 0) {
          sceKernelStartThread(client_id, 4, &con);
      }
    sceKernelWaitThreadEnd(client_id, 0);
  }

done:
    err = sceNetInetClose(sockListen);

  return 0;
}
