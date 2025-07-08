/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 */



#ifndef _PSP_CFG_H
#define _PSP_CFG_H

#ifdef __cplusplus
extern "C"{
#endif

typedef struct mftpUser_t  {
    struct mftpUser_t *next;
    char               user[MAX_USER_LENGTH];
    char               password[MAX_PASS_LENGTH];
    char               root[MAX_PATH_LENGTH];
    
  } mftpUser_t;

  typedef struct mftpConfig_t {
    int            auth_required;
    mftpUser_t    *head_user;

  } mftpConfig_t;

  extern mftpConfig_t mftp_config;

  extern int psp_read_config(void);

  extern mftpUser_t* cfg_get_user(char *User, char *Password);

#ifdef __cplusplus
}
#endif

# endif
