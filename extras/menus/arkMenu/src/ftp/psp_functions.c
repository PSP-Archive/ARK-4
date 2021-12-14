#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include "psp_functions.h"
#include "ftpsp.h"

int cmd_USER_func(struct ftpsp_client *client)
{
    client_send_ctrl_msg(client, "331 Username ok, need password");
    return 1;
}

int cmd_PASS_func(struct ftpsp_client *client)
{
    client_send_ctrl_msg(client, "230 User logged in");
    return 1;   
}

int cmd_SYST_func(struct ftpsp_client *client)
{
    client_send_ctrl_msg(client, "215 UNIX Type: L8");
    return 1;
}

int cmd_FEAT_func(struct ftpsp_client *client)
{
    client_send_ctrl_msg(client, "502 Error: command not implemented");  
    return 1;
}

int cmd_NOOP_func(struct ftpsp_client *client)
{
    client_send_ctrl_msg(client, "200 Command okay.");  
    return 1;
}


int cmd_PWD_func(struct ftpsp_client *client)
{
    char msg[strlen(client->cur_path) + 32];
    sprintf(msg, "257 \"%s\" is current directory.", client->cur_path);
    client_send_ctrl_msg(client, msg);
    return 1;
}

int cmd_QUIT_func(struct ftpsp_client *client)
{
    client_send_ctrl_msg(client, "221 Quit");
    return 1;
}

static void parse_format_control(struct ftpsp_client *client, int n_args, char *format_control)
{
    if (n_args < 2) { //Default is 'N'
        client->format_control = FTPSP_FMT_CTRL_NON_PRINT;
    } else {
        if (strlen(format_control) == 1) {
            switch (format_control[0]) {
            default:
            case 'N':
                client->format_control = FTPSP_FMT_CTRL_NON_PRINT;
                break;
            case 'T':
                client->format_control = FTPSP_FMT_CTRL_TELNET;
                break;
            case 'C':
                client->format_control = FTPSP_FMT_CTRL_ASA_CARRIAGE;
                break;
            }
        } else {
            client->format_control = atoi(format_control);
        }   
    }
}

int cmd_TYPE_func(struct ftpsp_client *client)
{
    char data_type;
    char format_control[8];
    int n_args = sscanf(client->rd_buffer, "%*s %c %s", &data_type, format_control);
    
    if (n_args > 0) {
        switch(data_type) {
        case 'A':
            client->data_type = FTPSP_DATA_ASCII;
            parse_format_control(client, n_args, format_control);
            break;
        case 'I':
            client->data_type = FTPSP_DATA_IMAGE;
            break;
        case 'E':
            client->data_type = FTPSP_DATA_EBCDIC;
            parse_format_control(client, n_args, format_control);
            break;
        case 'L':
            client->data_type = FTPSP_DATA_LOCAL;
            parse_format_control(client, n_args, format_control);
        default:
            client_send_ctrl_msg(client, "504 Error: bad parameters?");
            return 0;
        }
        client_send_ctrl_msg(client, "200 Okay");
    } else {
        client_send_ctrl_msg(client, "504 Error: bad parameters?");
    }
    return 1;
}


int cmd_PASV_func(struct ftpsp_client *client)
{
    return ftpsp_start_pasv(client);    
}

extern int errno;

int cmd_PORT_func(struct ftpsp_client *client)
{
    int ip[4];
    int porthi, portlo;
    sscanf(client->rd_buffer, "%*s %d,%d,%d,%d,%d,%d",
           &ip[3], &ip[2], &ip[1], &ip[0], &porthi, &portlo);
           
    int port = porthi*256 + portlo;
    char ip_str[16];
    sprintf(ip_str, "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);
    struct in_addr addr;
    inet_pton(AF_INET, ip_str, &addr);
    
    
    printf("PORT connection to ip: %s  port: %d\n", ip_str);
    
    client->data_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in clientaddr;
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(port); 
    clientaddr.sin_addr = addr;

    if(connect(client->data_sock, (struct sockaddr *)&clientaddr, sizeof(struct sockaddr))==-1){ 
        printf("connect() error: %d\n", errno);
    }
    
    client->conn_mode = FTPSP_CONN_ACTIVE;
    client_send_ctrl_msg(client, "200 PORT command successful");
    return 1;
}


static const char *num_to_month[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static int parse_ls_format(char *out, int n, int dir, unsigned int file_size, int month_n, int day_n, int hour, int minute, char *filename)
{
    return snprintf (out, n, 
                    "%c%s 1 psp psp %d %s %-2d %02d:%02d %s\n",
                     dir?'d':'-',
                     dir?"rwxr-xr-x":"rw-r--r--",
                     file_size,
                     num_to_month[(month_n-1)%12],
                     day_n,
                     hour,
                     minute,
                     filename);
}


static int get_ms_path(char *ms_path, const char *path)
{
    return sprintf(ms_path, "ms0:%s", path);
}

static int send_LIST(struct ftpsp_client *client, const char *path)
{
    ftpsp_open_data(client);
    client_send_ctrl_msg(client, "150 Opening ASCII mode data transfer for LIST");
    
    char ms_path[PATH_MAX+4];
    get_ms_path(ms_path, path);
    //printf("LIST ms path: %s\n", ms_path);
    
    SceUID dir = sceIoDopen(ms_path);
    SceIoDirent dirent;
    memset(&dirent, 0, sizeof(dirent));
    
    while (sceIoDread(dir, &dirent) > 0) {
        parse_ls_format(client->wr_buffer, BUF_SIZE,
                        FIO_S_ISDIR(dirent.d_stat.st_mode),
                        dirent.d_stat.st_size,
                        dirent.d_stat.st_ctime.month,
                        dirent.d_stat.st_ctime.day,
                        dirent.d_stat.st_ctime.hour,
                        dirent.d_stat.st_ctime.minute,
                        dirent.d_name);
        client_send_data_msg(client, client->wr_buffer);
        memset(&dirent, 0, sizeof(dirent));
        memset(client->wr_buffer, 0, BUF_SIZE);
    }
    
    sceIoDclose(dir);
    client_send_ctrl_msg(client, "226 Transfer complete");
    ftpsp_close_data(client);
    return 1;
}

int cmd_LIST_func(struct ftpsp_client *client)
{
    char path[PATH_MAX];
    int n = sscanf(client->rd_buffer, "%*s %[^\r\n\t]", path);
    if (n > 0) {  /* Client specified a path */
        send_LIST(client, path);
    } else {      /* Use current path */
        send_LIST(client, client->cur_path);
    }
    return 1;   
}

int cmd_CWD_func(struct ftpsp_client *client)
{
    char path[PATH_MAX];
    int n = sscanf(client->rd_buffer, "%*s %[^\r\n\t]", path);
    if (n < 1) {
        client_send_ctrl_msg(client, "500 Syntax error, command unrecognized");
    } else {
        if (strchr(path, '/') == NULL) { //Change dir relative to current dir
            if (client->cur_path[strlen(client->cur_path) - 1] != '/')
                strcat(client->cur_path, "/");
            strcat(client->cur_path, path);
        } else {
            strcpy(client->cur_path, path);
        }
        client_send_ctrl_msg(client, "250 Requested file action okay, completed.");
    }
    return 1;
}


static int dir_up(const char *in, char *out)
{
    char *pch = strrchr(in, '/');
    if (pch && pch != in) {
        size_t s = pch - in;
        strncpy(out, in, s);
        out[s] = '\0';
    } else {
        strcpy(out, "/");
    }
    return 1;
}

int cmd_CDUP_func(struct ftpsp_client *client)
{
    int s_len = strlen(client->cur_path)+1;
    char buf[s_len];
    memcpy(buf, client->cur_path, s_len);
    dir_up(buf, client->cur_path);
    client_send_ctrl_msg(client, "200 Command okay.");
    return 1;
}


static int send_file(struct ftpsp_client *client, const char *path)
{
    char ms_path[PATH_MAX+4];
    get_ms_path(ms_path, path);
    //printf("RETR ms path: %s\n", ms_path);
    SceUID fd;
    if ((fd = sceIoOpen(ms_path, PSP_O_RDONLY, 0777)) >= 0) {
        ftpsp_open_data(client);
        client_send_ctrl_msg(client, "150 Opening Image mode data transfer");
        
        unsigned int bytes_read;
        while ((bytes_read = sceIoRead (fd, client->wr_buffer, BUF_SIZE)) > 0) {
            send(client->data_sock, client->wr_buffer, bytes_read, 0);
        }

        sceIoClose(fd);
        client_send_ctrl_msg(client, "226 Transfer completed");
        ftpsp_close_data(client);
        
    } else {
        client_send_ctrl_msg(client, "550 File Not Found");
    }
    return 1;
}

int cmd_RETR_func(struct ftpsp_client *client)
{
    char path[PATH_MAX];
    char cur_path[PATH_MAX];
    sscanf(client->rd_buffer, "%*[^ ] %[^\r\n\t]", path);
    strcpy(cur_path, client->cur_path);
    if (strchr(path, '/') == NULL) { //File relative to current dir
        if (cur_path[strlen(cur_path) - 1] != '/')
            strcat(cur_path, "/");
        strcat(cur_path, path);
    }
    //printf("RETR: %s\n", cur_path);
    send_file(client, cur_path);
    return 1;       
}

