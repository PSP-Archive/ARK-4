# ifndef PSP_FTPD_H
# define PSP_FTPD_H

#ifdef __cplusplus
extern "C"{
#endif

int ftpdServerLoop();
int ftpdInit();
int ftpdTerm();
int ftpdState();
char* mftpGetLastStatusMessage();

#ifdef __cplusplus
}
#endif
	
# endif
