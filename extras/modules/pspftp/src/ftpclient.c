#include <pspkernel.h>
#include <pspwlan.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspnet_apctl.h>

#include "ftpclient.h"
#include "sutils.h"

// ftp has/has not been exited
int ftpquit = 0;

static int status;    // Current FTP State

// current downloaded/uploaded size
long ftpretrsize = 0;
long ftpstorsize = 0;

MclientConnection* con;                // Our Main Connection
int WaitForConnect = 0;                // Waiting for a Connection Request From Client Software
//status = STATUS_NOTCONNECTED;        // Current State - Initially NOT_CONNECTED

SceUID ftp_thread = -1;

extern uint16_t htons(uint16_t hostshort);

void debugTitle(const char* x){

}

void debugFooter(){

}

/*
int ftpGetWLANSwitch(void) {
    if ( sceWlanGetSwitchState() == 0 ) {
        return 0;
    } else {
        return 1;
    }
}

int ftpGetWLANPower(void) {
    if ( sceWlanDevIsPowerOn() == 0 ) {
        return 0;
    } else {
        return 1;
    }
}

int ftpGetEtherAddr(char *etherAddr) {
    int retVal;
    char sVal[7];
    memset(sVal, 0, 7);
    
    retVal = sceWlanGetEtherAddr(sVal);
    
    if ( retVal == 0 ) {
        sprintf(etherAddr, "%02X:%02X:%02X:%02X:%02X:%02X", sVal[0], sVal[1], sVal[2], sVal[3], sVal[4], sVal[5]);
        return 0;
    } else {
        strcpy(etherAddr, "Unknown Address");
        return 1;
    }
}
*/

long int ftpCurrentRETR(void) {
    return ftpretrsize;
}

long int ftpCurrentSTOR(void) {
    return ftpstorsize;
}

int ftpStatus(void) {
    return status;
}

int ftpExited(void) {
    return ftpquit;
}

int ftpTimeoutIS(int waitstatus, int sectimeout) {
    // waits until a specified status is no longer true and returns -1 if it does not in specified timeout
    int mscount = 0;
    while ( status == waitstatus && ftpExited() == 0 ) {
        sceKernelDelayThread(50*1000);
        mscount = mscount + 50;
        if ( mscount >= (sectimeout * 1000)) {
            // timeout reached
            status = STATUS_IDLE;
            return -1;
        }
    }
    if (ftpquit == 0 ) {
        return 0;    
    } else {
        return -1;
    }    
}

int ftpTimeoutISNOT(int waitstatus, int sectimeout) {
    // waits for a specific status to occur and returns -1 if it does not in specified timeout
    int mscount = 0;
    while ( status != waitstatus && ftpExited() == 0 ) {
        sceKernelDelayThread(50*1000);
        mscount = mscount + 50;
        if ( mscount >= (sectimeout * 1000)) {
            // timeout reached
            status = STATUS_IDLE;
            return -1;
        }
    }
    if (ftpquit == 0 ) {
        return 0;    
    } else {
        return -1;
    }    
}

// Sends a Response to the Server
void sendclientResponse(char* s) {
    strcat(con->comBuffer, s);
    if (endsWith(con->comBuffer, "\n")) {
        sceNetInetSend(con->comSocket, con->comBuffer, strlen(con->comBuffer) , 0);
        strcpy(con->comBuffer, "");
    }
}

// Sends a Response with Line Break to the Server
void sendclientResponseLn(char* s) {
    strcat(con->comBuffer, s);
    strcat(con->comBuffer, "\r\n");
    sceNetInetSend(con->comSocket, con->comBuffer, strlen(con->comBuffer) , 0);
    strcpy(con->comBuffer, "");
}

// Sends Data to the Server
void sendclientData(char* s) {
    strcat(con->dataBuffer, s);
    if (endsWith(con->dataBuffer, "\n")) {
        sceNetInetSend(con->dataSocket, con->dataBuffer, strlen(con->dataBuffer) , 0);
        strcpy(con->dataBuffer, "");
    }
}

// Sends Data with Line Break to the Server
void sendclientDataLn(char* s) {
    strcat(con->dataBuffer, s);
    strcat(con->dataBuffer, "\r\n");
    sceNetInetSend(con->dataSocket, con->dataBuffer, strlen(con->dataBuffer) , 0);
    strcpy(con->dataBuffer, "");
}

// Closes any open sockets
int closeClientDataConnection(void) {
    int err=0;
    err |= sceNetInetClose(con->dataSocket);
    return 0;
    //if (err) return 0; else return 1;
}


// Begins Opening the Data Connection
int openClientDataConnection(void) {

u32 err;

    // close any previous data connection
    closeClientDataConnection();

    // Check if We Are Using PASV Mode
    if (con->usePASV == 1) {
        // Using PASV Mode
        
        // send PASV command
        status = STATUS_SENDPASV;
        sendclientResponseLn("PASV");
        
        // wait for return/failure of pasv command
        int ret = ftpTimeoutISNOT(STATUS_ENTERPASV, ERRTIMEOUT);
        if (ret < 0) {
            // timed out
            status = STATUS_IDLE;
            return -1;
        }
        
        if ( status == STATUS_ENTERPASV ) {
            // PASV info (ip/port) was received and parsed
        
            // ******* Connect
        
            // Create Connect Socket
            struct sockaddr_in addrConnect;
            memset(&addrConnect, 0, sizeof(struct sockaddr_in));

            SOCKET sockConnect = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
            if (sockConnect & 0x80000000) {
                //printf("[ERROR] Unable to create connect socket");
                return -1;
            }
        
            // Setup Connection Info
            addrConnect.sin_family = AF_INET;
            
            // get the port to use (portA (static number) * 256 + portB (incrementing number)
            con->dataPort = (con->data_portA * 256) + con->data_portB;
            
            // set port to use
            addrConnect.sin_port = htons(con->dataPort);
            
            // ip info not needed
            addrConnect.sin_addr[0] = 0;
            addrConnect.sin_addr[1] = 0;
            addrConnect.sin_addr[2] = 0;
            addrConnect.sin_addr[3] = 0;
    
            // Bind the Connect Socket
            err = sceNetInetBind(sockConnect, &addrConnect, sizeof(addrConnect));
            if (err != 0) {
                //printf("[ERROR] Unable to bind to connect socket");
                status = STATUS_IDLE;
                return -1;
            }
        
            // Save the Connect Socket for Later
            con->dataSocket = sockConnect;
        
        
            // Finished, Return to IDLE State
            status = STATUS_IDLE;
        } else {
            // unknown status
            status = STATUS_IDLE;
            return -1;
        }
        
    } else {
        // Using PORT Mode
        
        
        // increment port for next data connection (won't effect PASV)
        con->data_portB++;
    
        
        // get local ip info to pass with PORT command
        char ipinfo[32];
        strncpy(ipinfo, con->localip, 31);
        strReplaceChar(ipinfo, '.', ',');
        
        // Set Status to Sending Port
        status = STATUS_SENDPORT;

        // get port info to pass with PORT command
        static char portinfo[7];
        sprintf(portinfo, "%d,%d", con->data_portA, con->data_portB);
        
        // Send Port Info
        //printf("PORT %s,%s", ipinfo, portinfo);
        
        // pass PORT command to remote
        sendclientResponse("PORT ");
        sendclientResponse(ipinfo);
        sendclientResponse(",");
        sendclientResponseLn(portinfo);
        
        // wait for return/failure of port command
        int ret = ftpTimeoutIS(STATUS_SENDPORT, ERRTIMEOUT);
        if (ret < 0) {
            return 0;
        }
        
        // port command returned success/failure
        if ( status == STATUS_PORTSUCCESS ) {
            // PORT Successful
        } else {
            // PORT Failed, Return OpenDataConnection Failed
            status = STATUS_IDLE;
            return 0;
        }
        
        // Create Listen Socket
        struct sockaddr_in addrListen;
        SOCKET sockListen = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
        if (sockListen & 0x80000000) {
            //printf("::error::unable to create listen socket");
            status = STATUS_IDLE;
            return 0;
        }
        
        // Setup Connection Info
        addrListen.sin_family = AF_INET;
        
        // get the port to use (portA (static number) * 256 + portB (incrementing number)
        con->dataPort = (con->data_portA * 256) + con->data_portB;
        
        // set port to use
        addrListen.sin_port = htons(con->dataPort);
        
        // ip info not needed
        addrListen.sin_addr[0] = 0;
        addrListen.sin_addr[1] = 0;
        addrListen.sin_addr[2] = 0;
        addrListen.sin_addr[3] = 0;

        // Bind the Listen Socket
        err = sceNetInetBind(sockListen, &addrListen, sizeof(addrListen));
        if (err != 0) {
            //printf("::error::unable to bind socket");
            return 0;
        }
        
        // Listen to the Listen Socket
        err = sceNetInetListen(sockListen, 1);
        if (err != 0) {
            //printf("::error::unable to listen to socket");
            return 0;
        }

        // Save the Listen Socket for Later
        con->listenSocket = sockListen;
        
        // Finished, Return to IDLE State
        status = STATUS_IDLE;
        
    }
    // data connection listening
    
    // Now that the Local Data Connection is ready, and once you have
    // sent a request for data (i.e RETR, LIST), all that needs to be done is
    // 1: call startDataAccept to begin Accepting connections
    // 2: call startDataConnect to connect to Server
    // 3: begin Send/Recv Calls

    return 1;
}


// Begins Accepting a Connection from Remote
// NOTE: Make Sure to Open a Data Connection First
int startDataAccept(void) {
    // only needed if we are using PORT, not PASV?
    if ( con->usePASV == 0 ) {
        // Create Accept Socket
        
        struct sockaddr addrAccept;
        SOCKET sockClient;
        
            int cbAddrAccept;
    
            cbAddrAccept = sizeof(addrAccept);
            sockClient = sceNetInetAccept(con->listenSocket, &addrAccept, &cbAddrAccept);
            if (sockClient & 0x80000000) {
                //printf("[ERROR] Unable to create accept socket");
                return -1;
            }
            
            con->dataSocket = sockClient;
            // data socket accepted and set
    }
        return 0;
}

int startDataConnect(void) {
    u32 err;
    struct sockaddr_in addrPort;
    memset(&addrPort, 0, sizeof(struct sockaddr_in));

    addrPort.sin_size = sizeof(struct sockaddr_in);
    addrPort.sin_family = AF_INET;
    addrPort.sin_port = htons(con->dataPort);
    
    if ( con->usePASV == 0 ) {
        addrPort.sin_addr[0] = 0;
        addrPort.sin_addr[1] = 0;
        addrPort.sin_addr[2] = 0;
        addrPort.sin_addr[3] = 0;
    } else {
        addrPort.sin_addr[0] = con->remoteip[0];
        addrPort.sin_addr[1] = con->remoteip[1];
        addrPort.sin_addr[2] = con->remoteip[2];
        addrPort.sin_addr[3] = con->remoteip[3];
    }
    // connecting data port to remote
    err = sceNetInetConnect(con->dataSocket, &addrPort, sizeof(struct sockaddr_in));
    
    if (err) {
        return 0;
    } else {
        // data port connected to remote
        return 1;
    }
}

void ftpSetPASV(int enabled) {
    con->usePASV = enabled;
}

int ftpPASVEnabled(void) {
    return con->usePASV;
}

void ftpDisconnect(void) {
    if (status > STATUS_NOTCONNECTED) {
    
        //printf("QUIT");
        sendclientResponseLn("QUIT");
    }
    //sceNetInetClose(con->comSocket);
    status = STATUS_NOTCONNECTED;
}

void ftpCHMOD(char* file, char* perm) {
    //printf("CHMOD %s %s", perm, file);
    sendclientResponse("SITE CHMOD ");
    sendclientResponse(perm);
    sendclientResponse(" ");
    sendclientResponseLn(file);
    
    // TODO: return success/failure
    // 200 success
}

void ftpSYST(void) {
    //printf("SYST");
    sendclientResponseLn("SYST");
    //TODO: return system type
}

void ftpRMD(char* dir) {
    //printf("RMD %s", dir);
    sendclientResponse("RMD ");
    sendclientResponseLn(dir);
    //todo: check for return success/fail
}

void ftpMKD(char* dir) {
    //printf("MKD %s", dir);
    sendclientResponse("MKD ");
    sendclientResponseLn(dir);
    //todo: check for return success/fail
}

void ftpAPPE(char* dir) {
    //printf("MKD %s", dir);
    sendclientResponse("MKF ");
    sendclientResponseLn(dir);
    //todo: check for return success/fail
}

void ftpDELE(char* file) {
    //printf("DELE %s", file);
    sendclientResponse("DELE ");
    sendclientResponseLn(file);
    //todo: check for return success/fail
}

void ftpCWD(char* dir) {
    //printf("CWD %s", dir);
    sendclientResponse("CWD ");
    sendclientResponseLn(dir);
}

char* ftpPWD(void) {
    //printf("PWD");
    sendclientResponseLn("PWD");
    // TODO: return current directory
    return "/";
}

void ftpABOR(void) {
    //printf("ABOR");
    sendclientResponseLn("ABOR");
}


remoteDirent *ftpLIST(void) {
    int ret;
    // Wait Till Other Events are Finished
    ret = ftpTimeoutISNOT(STATUS_IDLE, ERRTIMEOUT);
    if ( ret < 0 ) return NULL;
    
    // Attempt to Open a Data Connection
    if ( openClientDataConnection() == 0 ) {
        // Unable to Open a Local Data Connection
        //printf("[ERROR] LIST - unable to open data connection");
        return NULL;
    } else {
        // Succesfully Opened Local Data Connection
        
        // Send Request for Remote File Listing
        //printf("LIST");
        sendclientResponseLn("LIST");
        
            
        // Wait for Data connection accepted for Transfer to Start
        if (con->usePASV == 0) {
            status = STATUS_TRANSFERWAIT;
            ret = ftpTimeoutIS(STATUS_TRANSFERWAIT, ERRTIMEOUT);
            if ( ret < 0 ) {
                // timed out waiting
                //printf("[ERROR] LIST timed out");
                
                // close data connection
                closeClientDataConnection();
            
                status = STATUS_IDLE;
                return NULL;
            }
        } else {
            // PASV mode
            status = STATUS_TRANSFERWAIT;
            
        }
        
            // Wait For Connection Request from Remote 
            startDataAccept();
            // Connect to Remote
            startDataConnect();
        
        if ( status == STATUS_TRANSFERFAILED ) {
            status = STATUS_IDLE;
            return NULL;
        } else {
            
            // Begin Receiving Data
            int c=0;
            
            static char buf[sizeof(remoteDirent)*2];
            memset(buf, 0, sizeof(buf));
            char tmp_buf[TRANSFER_BUFFER_SIZE];
            int totalBuf = 0;
            
            while ((c=sceNetInetRecv(con->dataSocket, (u8*)tmp_buf, TRANSFER_BUFFER_SIZE, 0))>0 && ftpquit == 0 ) {
                memcpy(&buf[totalBuf], tmp_buf, c);
                totalBuf += c;
            }
    
            if ( ftpquit == 1 ) {
                return NULL;
            }
            
            static remoteDirent dir;
            memset(&dir, 0, sizeof(remoteDirent));
            
            int currFile = 0;
            int currInfo = 0;
            int infoLen=0;
            int spaceCount = 0;
            
            int i=0;
            
            while ( i < totalBuf ) {
            
                // INFO ISN'T SEPERATED BY TABS??? checking for spaces instead
                
                if ( buf[i] == '\n' ) {
                    
                    // end the file info
                    dir.files[currFile].d_name[infoLen - 1] = '\0';
                    
                    // keep track of total files/dir
                    dir.totalCount++;
                    
                    if (FIO_SO_ISDIR(dir.files[currFile].st_attr)) {
                        dir.dirCount++;
                    } else if (FIO_SO_ISLNK(dir.files[currFile].st_attr)) {
                        dir.linkCount++;
                    } else if (FIO_SO_ISREG(dir.files[currFile].st_attr)) {
                        dir.fileCount++;
                    }
                    
                    // print out info
                    /*
                    if (FIO_SO_ISDIR(dir.files[currFile].st_attr)) {
                        //printf("* %s", dir.files[currFile].d_name, dir.files[currFile].st_size);
                    } else if (FIO_SO_ISLNK(dir.files[currFile].st_attr)) {
                        //printf("%s", dir.files[currFile].d_name, dir.files[currFile].st_size);
                    } else if (FIO_SO_ISREG(dir.files[currFile].st_attr)) {
                        //printf("%s (%s)", dir.files[currFile].d_name, dir.files[currFile].st_size);
                    }
                    */
                    
                    // clear info for next file
                    currFile++;
                    currInfo = 0;
                    infoLen = 0;
                    spaceCount = 0;
                    
                } else {
                    // split line info
                    if ( buf[i] == ' ' ) {
                        
                        //drw-rw-rw-   1 user     group           0 Jun 28 19:42 Apps
                        // currInfo: (0) permissions (1) owner info  (2) group info  (3) size  (4) date  (5) filename
                        
                        if ( infoLen > 0 ) {
                            // this makes sure that we have started getting the next info in the line (there is at least 1 character in the info
                            if ( currInfo == 0 ) {
                                // no spaces in permissions, so we reached the end
                                dir.files[currFile].perms[infoLen] = '\0';
                                if ( dir.files[currFile].perms[0] == 'd' ) {
                                    dir.files[currFile].st_attr = FIO_SO_IFDIR;
                                } else if ( dir.files[currFile].perms[0] == 'l' ) {
                                    dir.files[currFile].st_attr = FIO_SO_IFLNK;
                                } else if ( dir.files[currFile].perms[0] == '-' ) {
                                    dir.files[currFile].st_attr = FIO_SO_IFREG;
                                }
                                
                                // start next info
                                currInfo++;
                                infoLen = 0;
                                spaceCount = 0;
                            } else if ( currInfo == 1) {
                                if ( spaceCount == 1 ) {
                                    // reached the end of the owner info, only 1 space is in this info
                                    dir.files[currFile].owner[infoLen] = '\0';
                                    // start next info
                                    currInfo++;
                                    infoLen = 0;
                                    spaceCount = 0;
                                } else {
                                    // the space is part of the info, just add it
                                    dir.files[currFile].owner[infoLen] = buf[i];
                                    infoLen++;
                                    spaceCount++;
                                }
                            } else if ( currInfo == 2 ) {
                                // no spaces in group, so we reached the end
                                dir.files[currFile].group[infoLen] = '\0';
                                // start next info
                                currInfo++;
                                infoLen = 0;
                                spaceCount = 0;
                            } else if ( currInfo == 3 ) {
                                // no spaces in size, so we reached the end
                                dir.files[currFile].st_size[infoLen] = '\0';
                                // start next info
                                currInfo++;
                                infoLen = 0;
                                spaceCount = 0;
                            } else if ( currInfo == 4) {
                                if ( spaceCount == 2 ) {
                                    // reached the end of the date info, only 3 spaces in this info
                                    dir.files[currFile].st_ctime[infoLen] = '\0';
                                    // start next info
                                    currInfo++;
                                    infoLen = 0;
                                    spaceCount = 0;
                                } else {
                                    // the space is part of the info, just add it
                                    dir.files[currFile].st_ctime[infoLen] = buf[i];
                                    infoLen++;
                                    spaceCount++;
                                }
                            } else if ( currInfo == 5 ) {
                                //unknown spaces in the filename,  the space is part of the info, just add it
                                dir.files[currFile].d_name[infoLen] = buf[i];
                                infoLen++;
                                spaceCount = 0;
                            }
                        }
                    } else {
                        // this is a regular character, just add it to the current info
                        
                        if ( currInfo == 0 ) {
                            dir.files[currFile].perms[infoLen] = buf[i];
                        } else if ( currInfo == 1 ) {
                            dir.files[currFile].owner[infoLen] = buf[i];
                        } else if ( currInfo == 2 ) {
                            dir.files[currFile].group[infoLen] = buf[i];
                        } else if ( currInfo == 3 ) {
                            dir.files[currFile].st_size[infoLen] = buf[i];
                        } else if ( currInfo == 4 ) {
                            dir.files[currFile].st_ctime[infoLen] = buf[i];
                        } else if ( currInfo == 5 ) {
                            dir.files[currFile].d_name[infoLen] = buf[i];
                        }
                        
                        infoLen++;
                        
                    }
                    //
                }
                i++;
            }
    
            // close data connection
            closeClientDataConnection();
            
            status = STATUS_IDLE;
            // success
            return &dir;
        }
    }
}


int ftpRETR(char* localdir, char* file) {
    int ret;

    ret = ftpTimeoutISNOT(STATUS_IDLE, ERRTIMEOUT);
    if ( ret < 0 ) return -1;
    
    
    //printf("TYPE I");
    sendclientResponseLn("TYPE I");
    
    if ( openClientDataConnection() == 0 ) {
        //printf("[ERROR] RETR - Unable to open data connection");
        status = STATUS_IDLE;
        return 0;
    } else {
        // succesfully opened data connection
        
        // setup file info
        char filePath[256];
        strcpy(filePath, localdir);
        strcat(filePath, file);
            
        // send request for file
        //printf("RETR %s (%s)", file, filePath);
        sendclientResponse("RETR ");
        sendclientResponseLn(file);

        // ****
        // HAD ACCEPT AND CONNECT HERE    
        // ****
            
        // setup buffer
        u8* buff[TRANSFER_BUFFER_SIZE];
    
        // Wait for Data connection accepted for Transfer to Start
        if (con->usePASV == 0) {
            status = STATUS_TRANSFERWAIT;
            ret = ftpTimeoutIS(STATUS_TRANSFERWAIT, ERRTIMEOUT);
            if ( ret < 0 ) {
                // timed out waiting
                //printf("[ERROR] RETR timed out");
                
                // close data connection
                closeClientDataConnection();
            
                status = STATUS_IDLE;
                return -1;
            }
        } else {
            // PASV mode
            status = STATUS_TRANSFERWAIT;
            
        }
        
        // begin server data accept/connect
        startDataAccept();
        startDataConnect();
        
        if ( status >= STATUS_TRANSFERFAILED ) {
            //printf("[ERROR] RETR failed (file not found?)");
            // file not found on server probably
            // close data connection
            closeClientDataConnection();
            status = STATUS_IDLE;
            return -1;
        } else {
        
            // open file for storing
            int fdFile = sceIoOpen(filePath, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
        
            //call SIZE for tracking progress
    
            // retrieve file
            int c=0;
            int dbytes = 0;
            while ((c=sceNetInetRecv(con->dataSocket, (u8*)buff, TRANSFER_BUFFER_SIZE, 0))>0  && ftpquit == 0 ) {
                // store the amount of bytes that has downloaded;
                ftpretrsize+=c;
                
                // redraw transfer every once and a while
                dbytes++;
                if ( dbytes >= 10 ) {
                    debugFooter();
                    dbytes = 0;
                }
                
                sceIoWrite(fdFile, buff, c);
            }
    
            // close file
            sceIoClose(fdFile);
            
            // TODO: if ftp prog was quit, del unfinished file
            
            // reset size of retr file download to 0
            ftpretrsize = 0;
            
            // close data connection
            closeClientDataConnection();
            
            status = STATUS_IDLE;
            
            // redraw footer
            debugFooter();
            
            if ( ftpquit == 1 ) {
                // quit while RETR was going
                return -1;
            } else {
                // RETR successful
                return 0;
            }
        }
    }
}

// Uploads a File to the Remote System
int ftpSTOR(char* localdir, char* file) {
    int ret;    // return value
    
    // wait till other events are finished
    ret = ftpTimeoutISNOT(STATUS_IDLE, ERRTIMEOUT);
    if ( ret < 0 ) return -1;
    
    // Send TYPE command for File Type (needed?)
    //printf("TYPE I");
    sendclientResponseLn("TYPE I");
    
    // TODO: call SIZE to check for existing file, for resuming uploads/percent complete
        
    // Attempt to Open a Data Connection
    if ( openClientDataConnection() == 0 ) {
        // Unable to Open a Local Data Connection
        //printf("[ERROR] STOR - Unable to open data connection");
        status = STATUS_IDLE;
        return -1;
    } else {
        // Succesfully Opened Local Data Connection

        
        // Setup File path and name
        char filePath[256];
        strcpy(filePath, localdir);
        strcat(filePath, file);
        
        // Check if this is a file or not
        SceIoStat fileStats;
        sceIoGetstat(filePath, &fileStats);

        if (FIO_SO_ISREG(fileStats.st_attr)) {
            // this is a file

            // TODO:
            // *** Need to move out of this if statement, once sending directories / aborting transfers is in place
            // *** from HERE 
            
            // Send Request to Retrieve File
            //printf("STOR %s", file);
            sendclientResponse("STOR ");
            sendclientResponseLn(file);
        
            // Wait for Data connection accepted for Transfer to Start
            if (con->usePASV == 0) {
                status = STATUS_TRANSFERWAIT;
                ret = ftpTimeoutIS(STATUS_TRANSFERWAIT, ERRTIMEOUT);
                if ( ret < 0 ) {
                    // timed out waiting
                    //printf("[ERROR] STOR timed out");
                    
                    // close data connection
                    closeClientDataConnection();
                
                    status = STATUS_IDLE;
                    return -1;
                }
            } else {
                // PASV mode
                status = STATUS_TRANSFERWAIT;
            }
            
                // Wait For Connection Request from Remote
                startDataAccept();
                // Connect to Remote
                startDataConnect();
            
            // Make sure that our request didn't fail
            if ( status == STATUS_TRANSFERFAILED ) {
                // Transfer failed, no permission
        
                // close data connection
                closeClientDataConnection();
                
                //printf("[ERROR] STOR - Transfer Failed (Permission?)");
                status = STATUS_IDLE;
                return -1;
            } else {
                        
                // TODO: Allow aborts
                // TODO: check if directory for directory transfers
                // TODO: setup multiple transfers to send recursive folders/files
                
                // open file for sending
                int fdFile = sceIoOpen(filePath, PSP_O_RDONLY, 0777);
    
                // Start Retrieving the File
                char* buf[TRANSFER_BUFFER_SIZE];
                int c=0;
                int dbytes = 0;
                while ((c=sceIoRead(fdFile, buf, TRANSFER_BUFFER_SIZE))>0 && ftpquit == 0) {
                    ftpstorsize+=c;
                    // redraw transfer every once and a while
                    dbytes++;
                    if ( dbytes >= 10 ) {
                        debugFooter();
                        dbytes = 0;
                    }
                    sceNetInetSend(con->dataSocket, buf, c , 0);
                }
                
                sceIoClose(fdFile);
                if (ftpquit == 0 ) {
                    //printf("STOR successful");
                }
                
                // reset size of stor file uploaded to 0
                ftpstorsize = 0;
                
                // close data connection
                closeClientDataConnection();
                return 0;
            }
        } else {
            //printf("[ERROR] STOR - Transfer Failed (File Missing? Permission? Dir?)");
            // this is a Directory? ?? no permission file? file missing?
        }

        // TODO: Supposedly when 226 is returned, then the connection is closed, but what if we don't get 226?

        // Return Status to IDLE
        status = STATUS_IDLE;
        
        // redraw footer
        debugFooter();
            
        
        return 0;
    }
}

int ftpLogin(char* user, char* pass) {
    int ret;    // return value
    
    //waiting for user request
    ret = ftpTimeoutISNOT(STATUS_WAITFORUSER, ERRTIMEOUT);
    if ( ret < 0 ) {
        status = STATUS_IDLE;
        return -1;
    }

    con->username = user;
    con->password = pass;
    
    // Send Username
    //printf("USER %s", user);
    sendclientResponse("USER ");
    sendclientResponseLn(user);
    
    // wait for PASS request
    ret = ftpTimeoutISNOT(STATUS_WAITFORPASS, ERRTIMEOUT);
    if ( ret < 0 ) {
        status = STATUS_IDLE;
        return -1;
    }
    
    // Send Password
    //printf("PASS %s", pass);
    sendclientResponse("PASS ");
    sendclientResponseLn(pass);

    // wait for login confirmation
    ret = ftpTimeoutISNOT(STATUS_LOGGEDIN, ERRTIMEOUT);
    if ( ret < 0 ) {
        status = STATUS_IDLE;
        return -1;
    }
    
    debugTitle("Logged in");
    debugFooter();
    
    status = STATUS_IDLE;
    return 1;
}

// Processes the commands retrieved from the server
int ftpDispatch(char* command) {
    int ret = 0;
    if (strlen(command)>0) {
        status = STATUS_PROCESSCMD;
        if (strStartsWith(command, "150 ") || strStartsWith(command, "150")) {
            // Data connection accepted
            status = STATUS_TRANSFERRING;
            ret = 150;
        } else if (strStartsWith(command, "200 ") || strStartsWith(command, "200")) {
            // Noop - Port command successful
            if (strStartsWith(command, "200 Port")) {
                status = STATUS_PORTSUCCESS;
            } else {
                status = STATUS_IDLE;
            }
            ret = 200;
        } else if (strStartsWith(command, "211 ") || strStartsWith(command, "211")) {
            // returns extensions supported
            // sends 211- to start, 211 End. to end
            status = STATUS_IDLE;
            ret = 211;
        } else if (strStartsWith(command, "215 ") || strStartsWith(command, "215")) {
            // return from SYST
            status = STATUS_IDLE;
            ret = 215;
        } else if (strStartsWith(command, "220 ") || strStartsWith(command, "220")) {
            // server ready
            status = STATUS_WAITFORUSER;
            ret = 220;
        } else if (strStartsWith(command, "226 ") || strStartsWith(command, "226")) {
            // transfer finished (OK  or  Aborted)
            closeClientDataConnection();
            status = STATUS_IDLE;
            ret = 226;
        } else if (strStartsWith(command, "227 ") || strStartsWith(command, "227")) {
            // Received PASV info
            //227 Entering Passive Mode (192,168,0,103,79,185)
            int params[6];
            char decimByte[4];
            char* pDecimByte=decimByte;
            char* pParams=skipWS(&command[27]);
        
            int state=0; int err=0; int nbParams=0;
            do {
                if (state==0 && *pParams>='0' && *pParams<='9') {
                    // if this is a # from 0-9
                    state=1;
                    pParams--;
                } else if (state==1 && *pParams>='0' && *pParams<='9') {
                    // if we have found a number already and this is a # from 0-9
                    if (pDecimByte-decimByte<=2) {
                        // set the next # in the char array
                        *(pDecimByte++)=*pParams;
                    } else {
                        err=1;
                    }
                } else if (state==1 && (*pParams==',' || *pParams==')' || *pParams==0) && nbParams<6) {
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

            // save data port info for later use
            con->data_portA = (unsigned char) params[4];
            con->data_portB = (unsigned char) params[5];
            
            // Entering PASV mode
            status = STATUS_ENTERPASV;
            ret = 227;
        } else if (strStartsWith(command, "230 ") || strStartsWith(command, "230")) {
            //login successful
            status = STATUS_LOGGEDIN;
            ret = 230;
        } else if (strStartsWith(command, "250 ") || strStartsWith(command, "250")) {
            // comand success (CWD/RMDIR/MKDIR)
            status = STATUS_IDLE;
            ret = 250;
        } else if (strStartsWith(command, "257 ") || strStartsWith(command, "257")) {
            //return of PWD
            // TODO: parse current dir
            status = STATUS_IDLE;
            ret = 257;
        } else if (strStartsWith(command, "331 ") || strStartsWith(command, "331")) {
            // waiting for password
            status = STATUS_WAITFORPASS;
            ret = 331;
        } else if (strStartsWith(command, "421 ") || strStartsWith(command, "421")) {
            // disconnected
            //printf("Server closed connection.");
            ret = 421;
            ftpDisconnect();
        } else if (strStartsWith(command, "425 ") || strStartsWith(command, "425")) {
            // unable to open data connection
            status = STATUS_TRANSFERFAILED;
            ret = 425;
        } else if (strStartsWith(command, "426 ") || strStartsWith(command, "426")) {
            // RETR aborted
            // TODO: add aborted status?
            ret = 426;
            ftpDisconnect();
        } else if (strStartsWith(command, "450 ") || strStartsWith(command, "450")) {
            // 450: Unable to Delete file
            status = STATUS_IDLE;
            ret = 450;
        } else if (strStartsWith(command, "500 ") || strStartsWith(command, "500")) {
            // failed port?
            status = STATUS_IDLE;
            ret = 500;
        } else if (strStartsWith(command, "550 ") || strStartsWith(command, "550")) {
            // 550: Cannot STOR. No permission. File not found on server
            status = STATUS_TRANSFERFAILED;
            ret = 550;
        } else {
            // Unimplemented
            //printf("~~ %s", command);
            status = STATUS_IDLE;
            return 999;
        }
    } else {
        status = STATUS_IDLE;
        ret = -1;
    }
    return ret;
}


// Waits for data to arrive from the server and process it
void ftpHandleResponses(void) {
            
    // send - receive data
    char readBuffer[1024];
    char lineBuffer[1024];
    int lineLen=0;
    int errLoop = 0;
    
    while(errLoop >= 0 && ftpquit == 0) {
        
        // waiting for data
        int nb = sceNetInetRecv(con->comSocket, (u8*)readBuffer, 1024, 0);

        if (nb <= 0) {
            // nothing to recv
            break;
        }

        int i=0;
        while (i<nb) {
            if (readBuffer[i]!='\r') {
                lineBuffer[lineLen++]=readBuffer[i];
                if (readBuffer[i]=='\n' || lineLen==1024) {
                    lineBuffer[--lineLen]=0;
                    
                    char* response=skipWS(lineBuffer);
                    trimEndingWS(response);
                    //printf("%s", response);
                    if ((errLoop = ftpDispatch(response)) < 0) {
                        //printf("[ERROR] Server Response is %s",response);
                        break;
                    }
                    lineLen=0;
                }
            }
            i++;
        }
    }
    // disconnected - recv loop finished
    ftpDisconnect();
}

int ftpConnected(void) {
    if ( status > STATUS_NOTCONNECTED ) {
        return 0;
    } else {
        return -1;
    }
}

int ftpLoggedIn(void) {
    if ( status >= STATUS_LOGGEDIN ) {
        return 1;
    } else {
        return 0;
    }
}

int mainThread(SceSize args, void *argp) {
    while ( WaitForConnect == 0 && ftpquit == 0) {
        sceKernelDelayThread(50*1000);
    }
    // todo make sure we are connected
    ftpHandleResponses();
    return 0;
}

void ftpInit(){
    // allocate connection info
    con = (MclientConnection*) malloc(sizeof(MclientConnection));
    con->dataSocket = 0;
    con->usePASV = 1;
    con->data_portA = 16;
    con->data_portB = 115;
    con->netconn = 1;
    
    memset(con->comBuffer, 0, 1024);
    memset(con->dataBuffer, 0, 1024);
    memset(con->localip, 0, 4);
    memset(con->remoteip, 0, 4);
    
    ftp_thread = sceKernelCreateThread("THREAD_FTP_CLIENTLOOP", mainThread, 0x18, 0x10000, 0, NULL);
    if(ftp_thread >= 0) {
        sceKernelStartThread(ftp_thread, 0, 0);
    } else {
        //printf("[ERROR] Impossible to create client thread.\n");
    }
}

void ftpClean(){
    free(con);
    con = NULL;
    sceKernelTerminateDeleteThread(ftp_thread);
}

int ftpConnect(char* ip, int port) {
    
    struct sockaddr_in addrTo;
    SOCKET sock;
    
    //printf("Get IP Address\n");
    if (sceNetApctlGetInfo(8, (union SceNetApctlInfo*)con->localip) != 0) {
        //printf("[ERROR] Impossible to get IP address of the PSP.\n");
        return 0;
    }
    //printf("Go IP: %s\n", con->localip);
    
    // parse remote ip address from string
    const char delimiters[] = ".";
    char* remoteip;
    remoteip = strtok(ip, delimiters);
    con->remoteip[0] = atoi(remoteip);
    remoteip = strtok(NULL, delimiters);
    con->remoteip[1] = atoi(remoteip);
    remoteip = strtok(NULL, delimiters);
    con->remoteip[2] = atoi(remoteip);
    remoteip = strtok(NULL, delimiters);
    con->remoteip[3] = atoi(remoteip);

    //printf("Opening socket\n");
    
    // create socket
    
    sock = sceNetInetSocket(AF_INET, SOCK_STREAM, 0);
    if (sock & 0x80000000) {
        //printf("[ERROR] Unable to create socket\n");
        return 0;
    }
    
    //printf("Socket open\n");
    
    con->comSocket=sock;
    
    addrTo.sin_family = AF_INET;
    addrTo.sin_port = htons(port);
    addrTo.sin_addr[0] = con->remoteip[0];
    addrTo.sin_addr[1] = con->remoteip[1];
    addrTo.sin_addr[2] = con->remoteip[2];
    addrTo.sin_addr[3] = con->remoteip[3];
    
    //printf("Connecting to %d.%d.%d.%d:%d\n", (unsigned char) con->remoteip[0], (unsigned char) con->remoteip[1], (unsigned char) con->remoteip[2], (unsigned char) con->remoteip[3], port);
    
    int err = sceNetInetConnect(sock, &addrTo, sizeof(addrTo));
    
    WaitForConnect = 1;
    if (err) {
        //printf("[ERROR] Unable to connect to server\n");
        return 0;
    } else {
        //printf("Connected\n");
        return 1;
    }
}
