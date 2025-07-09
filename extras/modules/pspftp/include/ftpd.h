# ifndef PSP_FTPD_H
# define PSP_FTPD_H

#ifdef __cplusplus
extern "C"{
#endif

extern void ftpdSetDevice(char* device);

extern char* ftpdGetDevice();

extern void ftpdSetMsgHandler(void (*handler)(const char*));

extern int ftpdLoop(SceSize argc, void *argv);

extern int ftpdExitHandler(SceSize argc, void *argv);

#ifdef __cplusplus
}
#endif

# endif
