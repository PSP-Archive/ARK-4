	.set noreorder

#include "pspimport.s"

// Build files
// SysclibForUser_XXXX.o

#ifdef F_SysclibForUser_0000
	IMPORT_START "SysclibForUser",0x40090000
#endif
#ifdef F_SysclibForUser_0001
	IMPORT_FUNC  "SysclibForUser",0x4C0FE24C,malloc
#endif
#ifdef F_SysclibForUser_0002
	IMPORT_FUNC  "SysclibForUser",0xA65E8BC4,free
#endif
#ifdef F_SysclibForUser_0003
	IMPORT_FUNC  "SysclibForUser",0xB19C939A,memalign
#endif
#ifdef F_SysclibForUser_0004
	IMPORT_FUNC  "SysclibForUser",0x7661E728,sprintf
#endif
#ifdef F_SysclibForUser_0005
	IMPORT_FUNC  "SysclibForUser",0x097049BD,bcopy
#endif

#ifdef F_SysclibForUser_0006
	IMPORT_FUNC  "SysclibForUser",0x10F3BB61,memset
#endif
#ifdef F_SysclibForUser_0007
	IMPORT_FUNC  "SysclibForUser",0x1493EBD9,wmemset
#endif
#ifdef F_SysclibForUser_0008
	IMPORT_FUNC  "SysclibForUser",0x1D83F344,atob
#endif
#ifdef F_SysclibForUser_0009
	IMPORT_FUNC  "SysclibForUser",0x3EC5BBF6,tolower
#endif
#ifdef F_SysclibForUser_0010
	IMPORT_FUNC  "SysclibForUser",0xCE2F7487,toupper
#endif
#ifdef F_SysclibForUser_0011
	IMPORT_FUNC  "SysclibForUser",0x7F8A6F23,bcmp
#endif
#ifdef F_SysclibForUser_0012
	IMPORT_FUNC  "SysclibForUser",0x86FEFCE9,bzero
#endif
#ifdef F_SysclibForUser_0013
	IMPORT_FUNC  "SysclibForUser",0x68A78817,memchr
#endif
#ifdef F_SysclibForUser_0014
	IMPORT_FUNC  "SysclibForUser",0x81D0D1F7,memcmp
#endif
#ifdef F_SysclibForUser_0015
	IMPORT_FUNC  "SysclibForUser",0xA48D2592,memmove
#endif
#ifdef F_SysclibForUser_0016
	IMPORT_FUNC  "SysclibForUser",0xAB7592FF,memcpy
#endif
#ifdef F_SysclibForUser_0017
	IMPORT_FUNC  "SysclibForUser",0xB1DC2AE8,strchr
#endif
#ifdef F_SysclibForUser_0018
	IMPORT_FUNC  "SysclibForUser",0xEC6F1CF2,strcpy
#endif
#ifdef F_SysclibForUser_0019
	IMPORT_FUNC  "SysclibForUser",0xB49A7697,strncpy
#endif
#ifdef F_SysclibForUser_0020
	IMPORT_FUNC  "SysclibForUser",0xC0AB8932,strcmp
#endif
#ifdef F_SysclibForUser_0021
	IMPORT_FUNC  "SysclibForUser",0x87F8D2DA,strtok
#endif
#ifdef F_SysclibForUser_0022
	IMPORT_FUNC  "SysclibForUser",0x89B79CB1,strcspn
#endif
#ifdef F_SysclibForUser_0023
	IMPORT_FUNC  "SysclibForUser",0x90C5573D,strnlen
#endif
#ifdef F_SysclibForUser_0024
	IMPORT_FUNC  "SysclibForUser",0x6A7900E1,strtoul
#endif
#ifdef F_SysclibForUser_0025
	IMPORT_FUNC  "SysclibForUser",0x7AB35214,strncmp
#endif
#ifdef F_SysclibForUser_0026
	IMPORT_FUNC  "SysclibForUser",0x983B00FB,lowerString
#endif
#ifdef F_SysclibForUser_0027
	IMPORT_FUNC  "SysclibForUser",0xAC7554DF,strncasecmp
#endif
#ifdef F_SysclibForUser_0028
	IMPORT_FUNC  "SysclibForUser",0x0D188658,strstr
#endif
#ifdef F_SysclibForUser_0029
	IMPORT_FUNC  "SysclibForUser",0x0DFB7B6C,strpbrk
#endif
#ifdef F_SysclibForUser_0030
	IMPORT_FUNC  "SysclibForUser",0x476FD94A,strcat
#endif
#ifdef F_SysclibForUser_0031
	IMPORT_FUNC  "SysclibForUser",0x47DD934D,strtol
#endif
#ifdef F_SysclibForUser_0032
	IMPORT_FUNC  "SysclibForUser",0x4C0E0274,strrchr
#endif
#ifdef F_SysclibForUser_0033
	IMPORT_FUNC  "SysclibForUser",0x52DF196C,strlen
#endif
#ifdef F_SysclibForUser_0034
	IMPORT_FUNC  "SysclibForUser",0x62AE052F,strspn
#endif
#ifdef F_SysclibForUser_0035
	IMPORT_FUNC  "SysclibForUser",0x1AB53A58,strtok_r
#endif
#ifdef F_SysclibForUser_0036
	IMPORT_FUNC  "SysclibForUser",0xBC7554DF,strcasecmp
#endif
