#ifndef NETWORK_H
#define NETWORK_H

typedef struct {
    unsigned int size;
    int message_lang;
    int ctrl_assign;
    int main_thread_priority;
    int sub_thread_priority;
    int font_thread_priority;
    int sound_thread_priority;
    int result; 
    int reserved1;
    int reserved2;
    int reserved3;
    int reserved4;
} SceUtilityParamBase;

typedef struct
{
    unsigned char group_name[8];
    unsigned int timeout;
} SceUtilityNetconfAdhocParam;

typedef struct
{
    SceUtilityParamBase base;
    int type;
    SceUtilityNetconfAdhocParam * adhoc_param;
    unsigned int browser_available;
    unsigned int browser_flag;
    unsigned int wifisvc_available;
} SceUtilityNetconfParam;

extern int initializeNetwork(void);
extern int connect_to_apctl(void);
extern int shutdownNetwork();
extern char* resolveHostAddress(char*);
extern int wget(char* url, char* saveAs, SceULong64* cur_download=NULL, SceULong64* max_download=NULL);

#endif
