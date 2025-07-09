#ifndef FTPCLIENT_H
#define FTPCLIENT_H

#ifdef __cplusplus
extern "C"{
#endif

#include "my_socket.h"

#define TRANSFER_BUFFER_SIZE     4096    // Buffer size for transfer
#define MAX_FILES                 256        // MAX files in a list
#define ERRTIMEOUT                15        // TIMEOUT in seconds

// TODO seperate states for local and remote to prevent unknown status change while waiting for a specific status

// Various States the Connection May Be In
#define STATUS_NOTCONNECTED     0
#define STATUS_WAITFORUSER        1
#define STATUS_WAITFORPASS        2
#define STATUS_LOGGEDIN            3
#define STATUS_SENDPASV            4
#define STATUS_ENTERPASV        5
#define STATUS_SENDPORT            6
#define STATUS_PORTSUCCESS        7
#define STATUS_PROCESSCMD       8
#define STATUS_TRANSFERWAIT        9
#define STATUS_TRANSFERRING     10
#define STATUS_TRANSFERFAILED    11
#define STATUS_IDLE                12

// connection used
typedef struct MclientConnection {
    SOCKET comSocket;
    SOCKET listenSocket;
    SOCKET dataSocket;

    int netconn;
    int dataPort;
    int data_portA;
    int data_portB;

    char localip[32];
    char remoteip[4];

    char comBuffer[1024];
    char dataBuffer[1024];

    char* username;
    char* password;
    int usePASV;
} MclientConnection;

// file structure for remote files
typedef struct remoteFileent {
    char perms[10];
    char owner[10];
    char group[10];
    unsigned int st_attr;    // file type (DIR, LINK, FILE
    char st_size[12];        // size of file
    char d_name[256];        // name of file
    char st_ctime[12];        // creation time
} remoteFileent;

// directory structure for a directory listing
typedef struct remoteDirent {
    int totalCount;
    int dirCount;
    int fileCount;
    int linkCount;
    struct remoteFileent files[MAX_FILES];
} remoteDirent;

// helpful net functions
//int ftpGetWLANSwitch(void);                    // returns if WLAN switch is on or off
//int ftpGetWLANPower(void);                    // returns if WLAN Power save is on?
//int ftpGetEtherAddr(char *etherAddr);        // gets ethernet address, pass a pointer to a buffer of chars at least length 19

// implementation functions
//int startFTP(SceModuleInfo *modInfoPtr);    // starts the ftp thread loop and waits for a ftpConnect() call
void ftpInit();
void ftpClean();
int ftpConnect(char* ip, int port);            // connects to remote address
int ftpLogin(char* user, char* pass);        // called once connected, to login
void ftpDisconnect(void);                    // disconnects from remote if connected
//void quitFTP(void);                            // quits the ftp thread loop

int ftpStatus(void);                        // returns the current status of the client
int ftpExited(void);                        // returns if the client has exited the program
int ftpConnected(void);                        // returns if currently connected
int ftpLoggedIn(void);                        // returns 1 if currently logged in

long int ftpCurrentRETR(void);                // returns downloaded size of current download
long int ftpCurrentSTOR(void);                // returns downloaded size of current upload
void ftpSetPASV(int enabled);                    // enable/disable PASV
int ftpPASVEnabled(void);                        // returns if pasv is enabled

// ftp commands
int ftpRETR(char* location, char* file);    // download a file
int ftpSTOR(char* localdir, char* file);    // uploads a file
void ftpABOR(void);                            // sends ABOR command
void ftpCHMOD(char* file, char* perm);        // sets file permissions of a remote file
void ftpSYST(void);                            // returns system type of remote
void ftpRMD(char* dir);                        // removes a directory
void ftpMKD(char* dir);                        // makes a directory
void ftpAPPE(char* dir);                        // makes a file
void ftpDELE(char* file);                    // deletes a file
void ftpCWD(char* dir);                        // change directory
char* ftpPWD(void);                            // returns current directory
remoteDirent* ftpLIST(void);                // returns a list of files
// long int ftpSIZE(char* file);                        // gets the size of a remote file

#ifdef __cplusplus
}
#endif

#endif
