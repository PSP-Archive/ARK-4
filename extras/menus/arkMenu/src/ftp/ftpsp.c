#include <pspkernel.h>
#include <pspsdk.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "ftpsp.h"
#include "mutex.h"
#include "utils.h"
#include "psp_functions.h"

#define add_entry(name) {#name, cmd_##name##_func}
struct dispatch_entry dispatch_table[] = {
    add_entry(USER),
    add_entry(PASS),
    add_entry(SYST),
    add_entry(FEAT),
    add_entry(NOOP),
    add_entry(PWD),
    add_entry(QUIT),
    add_entry(TYPE),
    //add_entry(PASV),
    add_entry(PORT),
    add_entry(LIST),
    add_entry(CWD),
    add_entry(CDUP),
    add_entry(RETR),
    {"\0",   NULL},
};


       char psp_ip[16]    = {0};
static SceUID server_thid = -1;
static int server_sock    = -1;
static SceUID list_mutex  = -1;
static int run_server_th  =  0;
static struct ftpsp_client *client_list_head = NULL;

static void client_handle(struct ftpsp_client *client);
static struct ftpsp_client *client_create(int socket, struct sockaddr_in *sockaddr);
static void client_destroy(struct ftpsp_client *client);
static void client_self_destroy(struct ftpsp_client *client);

static dispatch_function get_dispatch_func(const char *cmd);

static void client_list_add(struct ftpsp_client *new_client);
static void client_list_delete(struct ftpsp_client *client);
static void client_list_remove_client(struct ftpsp_client *client);
static void client_list_clear();
static int  client_list_size();
static int  start_server_sock(int port);

static int server_thread(SceSize args, void *argp);
static int client_thread(SceSize args, void *argp);

#define printf(...) {char buf[256]; snprintf(buf, 256, __VA_ARGS__); if(msg_handler)msg_handler(buf);}

static void (*msg_handler)(char* msg) = NULL;

void setFtpMsgHandler(void* handler){
    msg_handler = handler;
}

int ftpsp_init()
{
    if (!start_server_sock(CTRL_PORT)) {
        printf("init error\n");
    }
    
    if (get_ip(psp_ip)) {
        printf("PSP IP is: %s\n", psp_ip);
    }
    
    list_mutex = sceKernelCreateMutex("ftpsp_list_mutex", PSP_MUTEX_ATTR_FIFO, 0, NULL);
    
    server_thid = sceKernelCreateThread("ftpsp_server_thread", server_thread, 0x20, 0x6000, PSP_THREAD_ATTR_USBWLAN, NULL);
    if (server_thid < 0) {
        printf("Error creating main thread: 0x%X\n", server_thid);
        return -1;
    } else {
        run_server_th = 1;
        sceKernelStartThread(server_thid, 0, NULL);
    }
    
    return 0;
}

int ftpsp_reset()
{
    ftpsp_shutdown();
    ftpsp_init();
    return 1;
}

int ftpsp_shutdown()
{
    run_server_th = 0;
    //sceKernelWaitThreadEnd(server_thid, NULL);
    sceKernelTerminateDeleteThread(server_thid);
    close(server_sock);
    client_list_clear();
    sceKernelDeleteMutex(list_mutex);
    return 1;
}


static int server_thread(SceSize args, void *argp)
{
    printf("Waiting for clients to connect...\n");
    while (run_server_th) {
        
        printf("BEFORE ACCEPT!!! size: %d\n", client_list_size());
        int client_sock = accept(server_sock, NULL, NULL);
        printf("AFTER ACCEPT!!! size: %d\n", client_list_size());

        struct sockaddr_in new_addr;
        socklen_t addr_len = sizeof(new_addr);
        getpeername(client_sock, (struct sockaddr *)&new_addr, &addr_len);   


        int port = ntohs(new_addr.sin_port);   
        uint32_t ip_n = ntohl(new_addr.sin_addr.s_addr);
        unsigned char *ip = (unsigned char*)&ip_n;

        printf("Connection from: %d.%d.%d.%d  port: %d\n", ip[3], ip[2], ip[1], ip[0], port);

        
        if (client_sock >= 0) {
            struct ftpsp_client *client = client_create(client_sock, &new_addr);
            struct ftpsp_thread_args th_args =  {
                                                .client = client
                                                };
            client->thid = sceKernelCreateThread("ftpsp_client_thread", client_thread, 0x12, 0x2000, PSP_THREAD_ATTR_USBWLAN, NULL);
            if (client->thid < 0) {
                printf("Error creating client thread: 0x%X\n", server_thid);
            } else {
                client_list_add(client);
                sceKernelStartThread(client->thid, sizeof(th_args), &th_args);
            }
        }
                          
        sceKernelDelayThread(1000);
    }
    printf("Exit main thread\n");
    sceKernelExitDeleteThread(0);
    return 0;
}


static int client_thread(SceSize args, void *argp)
{
    struct ftpsp_thread_args *th_args = (struct ftpsp_thread_args *)argp;
    struct ftpsp_client *client = th_args->client;
    
    printf("I'm a client!\n");
    client_send_ctrl_msg(client, "220 My body is ready!\n");
    
    while (client->run_thread) {
        client_handle(client);
        sceKernelDelayThread(1000);   
    }
    
    printf("Exiting client thread...\n");
    client_list_remove_client(client);
    client_self_destroy(client);
    sceKernelExitDeleteThread(0);
    return 0;   
}

static int get_command(const char *buffer, char *cmd_out)
{
    return sscanf(buffer, "%s", cmd_out);
}

static void client_handle(struct ftpsp_client *client)
{
    //Clean previous recv
    memset(client->rd_buffer, 0, BUF_SIZE);
    int bytes_recv = recv(client->ctrl_sock, client->rd_buffer, BUF_SIZE, 0);

    if (bytes_recv <= 0) {    /* Client disconnected  or read error*/
        printf("0 bytes recv\n");
        if (bytes_recv == 0) printf("client disconnected\n")
        else                 printf("read error\n");
        client_destroy(client);
        
    } else {
        char cmd[5];
        get_command(client->rd_buffer, cmd);
        //printf("command recv: %s\n", cmd);
        dispatch_function func = NULL;
        if ((func = get_dispatch_func(cmd))) {
            //printf("calling function: %p\n", func);
            func(client);
        } else { 
            client_send_ctrl_msg(client, "502 Error: command not implemented");  
        }
    }
}

/*
struct ftpsp_client {
    struct in_addr ip_addr;
    char rd_buffer[BUF_SIZE];
    char wr_buffer[BUF_SIZE];
    char cur_path[PATH_MAX];
    struct ftpsp_client *next;
    struct ftpsp_client *prev;
*/

static struct ftpsp_client *client_create(int socket, struct sockaddr_in *sockaddr)
{
    struct ftpsp_client *client = calloc(1, sizeof(struct ftpsp_client));
    client->ctrl_sock  =  socket;
    client->pasv_listener = -1;
    client->data_sock  = -1;
    client->conn_mode  =  FTPSP_CONN_NONE;
    client->thid       = -1;
    client->run_thread =  1;
    client->ip_addr    = sockaddr->sin_addr;
    memset(client->rd_buffer, 0, BUF_SIZE);
    memset(client->wr_buffer, 0, BUF_SIZE);
    memset(client->cur_path,  0, PATH_MAX);
    strcpy(client->cur_path, "/");
    client->next = NULL;
    client->prev = NULL;
    //inet_ntop(AF_INET, (void*)&sockaddr->sin_addr, client->ip, 16);
    //printf("new connection from: %s   port: %d\n", inet_ntoa(sockaddr->sin_addr), ntohs(sockaddr->sin_port));
    return client;
}


int ftpsp_start_pasv(struct ftpsp_client *client)
{
    if (client->conn_mode != FTPSP_CONN_NONE) {
        ftpsp_close_data(client);
    }

    /* Start passive connection listener */
    client->pasv_listener = socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    setsockopt(client->pasv_listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    struct sockaddr_in listener;
    socklen_t addrlen = sizeof(listener);
    getsockname(client->ctrl_sock, (struct sockaddr *)&listener, &addrlen);
    listener.sin_port = 0;

    bind(client->pasv_listener, (struct sockaddr *) &listener, sizeof(listener));
    listen(client->pasv_listener, 0);
    
    struct sockaddr_in pasv_addr;
    socklen_t pasv_len = sizeof(pasv_addr);
    getsockname(client->pasv_listener, (struct sockaddr *) &pasv_addr, &pasv_len);
    
    int pasv_port = ntohs(pasv_addr.sin_port);   
    uint32_t ip_n = ntohl(pasv_addr.sin_addr.s_addr);
    unsigned char *ip = (unsigned char*)&ip_n;

    int n = sprintf(client->wr_buffer, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
                    ip[3], ip[2], ip[1], ip[0], (pasv_port>>8) & 0xFF, pasv_port & 0xFF);
    client_send_ctrl_msg(client, client->wr_buffer);
    client->conn_mode = FTPSP_CONN_PASSIVE;
    return 1;
}

int ftpsp_stop_pasv(struct ftpsp_client *client)
{
    //shutdown(client->pasv_listener, SHUT_RDWR);
    close(client->pasv_listener);
    client->pasv_listener = -1;
    client->conn_mode = FTPSP_CONN_NONE;
    return 1;
}

int ftpsp_open_data(struct ftpsp_client *client)
{
    if (client->conn_mode == FTPSP_CONN_ACTIVE) {
        //Nothing?
    } else if (client->conn_mode == FTPSP_CONN_PASSIVE) {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        client->data_sock = accept(client->pasv_listener,  (struct sockaddr *)&addr, &len);

        int port = ntohs(addr.sin_port);
        uint32_t ip_n = ntohl(addr.sin_addr.s_addr);
        unsigned char *ip = (unsigned char*)&ip_n;

        printf("PASV Connection from: %d.%d.%d.%d  port: %d\n", ip[3], ip[2], ip[1], ip[0], port);   
    }
    return 1;
}

int ftpsp_close_data(struct ftpsp_client *client)
{
    if (client->conn_mode != FTPSP_CONN_NONE) {
        close(client->data_sock);
        client->data_sock = -1;
        if (client->conn_mode == FTPSP_CONN_PASSIVE) {
            ftpsp_stop_pasv(client);
        } else {
            client->conn_mode = FTPSP_CONN_NONE;
        }
    }
    return 1;
}

int client_send_ctrl_msg(struct ftpsp_client *client, const char *msg)
{
    int n = send(client->ctrl_sock, msg, strlen(msg), 0);
    return n + send(client->ctrl_sock, "\r\n", 2, 0);
}

int client_send_data_msg(struct ftpsp_client *client, const char *msg)
{
    int n = send(client->data_sock, msg, strlen(msg), 0);
    return n + send(client->data_sock, "\r\n", 2, 0);
}


static void client_destroy(struct ftpsp_client *client)
{
    client->run_thread = 0;
    /*
    //SceUInt timeout = 1000;
    if (client->thid >= 0) {
        int sceKernelWaitThreadEnd(client->thid, &timeout);
        //if (ret == SCE_KERNEL_ERROR_WAIT_TIMEOUT)
            sceKernelTerminateDeleteThread(client->thid);
    }
    client_self_destroy(client);
    */
}

static void client_self_destroy(struct ftpsp_client *client)
{
    if (client->ctrl_sock > 0)
        close(client->ctrl_sock);
    if (client->data_sock > 0)
        close(client->data_sock);
    if (client->pasv_listener > 0)
        close(client->pasv_listener);
    free(client);
}

/* Returns 1 if equal, 0 otherwise */
static int compare_cmd(const char *cmd1, const char *cmd2)
{
    int i = 0;
    while (cmd1[i] && cmd2[i] && cmd1[i] == cmd2[i]) {
        ++i;
    }
    return cmd1[i] == cmd2[i];
}

static dispatch_function get_dispatch_func(const char *cmd)
{
    int i;
    for(i = 0; dispatch_table[i].cmd && dispatch_table[i].func; ++i) {
        int r = compare_cmd(dispatch_table[i].cmd, cmd);
        //printf("comparing: \"%s\" with \"%s\" : %d\n", cmd, dispatch_table[i].cmd, r);
        if (r)
            return dispatch_table[i].func;
    }
    return NULL;
}

static void client_list_add(struct ftpsp_client *new_client)
{
    sceKernelLockMutex(list_mutex, 1, NULL);
    if (client_list_head) {
        client_list_head->next = new_client;
        new_client->prev = client_list_head;
        client_list_head = new_client;
    } else {
        client_list_head = new_client;
        new_client->prev = NULL;
    }
    new_client->next = NULL;
    sceKernelUnlockMutex(list_mutex, 1);
}

static void client_list_delete(struct ftpsp_client *client)
{
    client_list_remove_client(client);
    client_destroy(client);
}

static void client_list_remove_client(struct ftpsp_client *client)
{
    sceKernelLockMutex(list_mutex, 1, NULL);
    if (client->next) {
        if (client->prev) { /* Middle of the list */
            (client->next)->prev = client->prev;
            (client->prev)->next = client->next;
        } else {            /* Bottom of the list */
            (client->next)->prev = NULL;
        }
    } else {
        if (client->prev) { /* Head of the list */
            client_list_head = client->prev;
        } else {            /* Only one element left */
            client_list_head = NULL;
        }
    }
    sceKernelUnlockMutex(list_mutex, 1);
}

static void client_list_clear()
{
    while (client_list_head) {
        client_list_delete(client_list_head);
    }
}

static int client_list_size()
{
    sceKernelLockMutex(list_mutex, 1, NULL);
    int size = 0;
    struct ftpsp_client *p = client_list_head;
    while (p) {
        p = client_list_head->prev;
        ++size;
    }
    sceKernelUnlockMutex(list_mutex, 1);
    return size;
}

static int start_server_sock(int port)
{
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        printf("Error creating server socket: 0x%X", server_sock);
        return 0;   
    }
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port   = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int reuse = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    bind(server_sock, (struct sockaddr *) &server, sizeof(server));
    
    listen(server_sock, 128);
    return 1;
}
