#ifndef _FTP_H_
#define _FTP_H_

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syslimits.h>

extern char psp_ip[16];

#define BUF_SIZE 1024
#define CTRL_PORT 21

enum FTPSP_CONN_MODE {
    FTPSP_CONN_NONE,   //Before PASV or PORT
    FTPSP_CONN_ACTIVE,
    FTPSP_CONN_PASSIVE
};

enum FTPSP_DATA_TYPE {
    FTPSP_DATA_ASCII,
    FTPSP_DATA_EBCDIC,
    FTPSP_DATA_IMAGE, //Binary
    FTPSP_DATA_LOCAL
};

enum FTPSP_FORMAT_CONTROL {
    FTPSP_FMT_CTRL_NON_PRINT,
    FTPSP_FMT_CTRL_TELNET,
    FTPSP_FMT_CTRL_ASA_CARRIAGE
};

struct ftpsp_client {
    int ctrl_sock;
    int pasv_listener;
    int data_sock;
    int conn_mode;
    int data_type;
    int format_control;
    SceUID thid;
    int run_thread;
    struct in_addr ip_addr;
    char rd_buffer[BUF_SIZE];
    char wr_buffer[BUF_SIZE];
    char cur_path[PATH_MAX];
    /* List */
    struct ftpsp_client *next;
    struct ftpsp_client *prev;
};

struct ftpsp_thread_args {
    struct ftpsp_client *client;
};

typedef int (*dispatch_function)(struct ftpsp_client*);

struct dispatch_entry {
    char cmd[5];
    dispatch_function func;
};

int ftpsp_init();
int ftpsp_reset();
int ftpsp_shutdown();


int ftpsp_start_pasv(struct ftpsp_client *client);
int ftpsp_stop_pasv(struct ftpsp_client *client);

inline
int ftpsp_open_data(struct ftpsp_client *client);
inline
int ftpsp_close_data(struct ftpsp_client *client);

inline
int client_send_ctrl_msg(struct ftpsp_client *client, const char *msg);
inline
int client_send_data_msg(struct ftpsp_client *client, const char *msg);


/*
int read_command(struct ftpsp_command *ftpsp_cmd);
int send_command(const char *cmd, const char *data);
int send_ftpsp_command(const struct ftpsp_command *ftpsp_cmd);
int parse_command(const char *buf, struct ftpsp_command *ftpsp_cmd);
int serialize_ftpsp_command(char *buf, const struct ftpsp_command *ftpsp_cmd);
int serialize_command(char *buf, const char *cmd, const char *data);
void build_command(struct ftpsp_command *ftpsp_cmd, const char *cmd, const char *data);

int cmd_to_code(const char *cmd);
const char *code_to_cmd(int code);
int start_server_socket(int port);
int send_text(int socket, char *text);
int recv_data(int socket);
*/

#endif 
