#ifndef _PSP_FUNCTIONS_H_
#define _PSP_FUNCTIONS_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "utils.h"
#include "ftpsp.h"



int cmd_USER_func(struct ftpsp_client *client);
int cmd_PASS_func(struct ftpsp_client *client);
int cmd_SYST_func(struct ftpsp_client *client);
int cmd_FEAT_func(struct ftpsp_client *client);
int cmd_NOOP_func(struct ftpsp_client *client);
int cmd_PWD_func(struct ftpsp_client *client);
int cmd_QUIT_func(struct ftpsp_client *client);
int cmd_TYPE_func(struct ftpsp_client *client);
int cmd_PASV_func(struct ftpsp_client *client);
int cmd_PORT_func(struct ftpsp_client *client);
int cmd_LIST_func(struct ftpsp_client *client);
int cmd_CWD_func(struct ftpsp_client *client);
int cmd_CDUP_func(struct ftpsp_client *client);
int cmd_RETR_func(struct ftpsp_client *client);

#endif 
