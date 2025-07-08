	.set noreorder

#include "pspimport.s"

// Build files
// SysclibForUser_XXXX.o

#ifdef F_SysclibForUser_0000
	IMPORT_START "SysclibForUser",0x40090000
#endif
#ifdef F_SysclibForUser_0001
	IMPORT_FUNC  "SysclibForUser",0xBC7554DF,strcasecmp
#endif
#ifdef F_SysclibForUser_0002
	IMPORT_FUNC  "SysclibForUser",0xAC7554DF,strncasecmp
#endif
#ifdef F_SysclibForUser_0003
	IMPORT_FUNC  "SysclibForUser",0x983B00FB,lowerString
#endif
#ifdef F_SysclibForUser_0004
	IMPORT_FUNC  "SysclibForUser",0x7ED93D95,strtok
#endif
#ifdef F_SysclibForUser_0005
	IMPORT_FUNC  "SysclibForUser",0x7DF95413,strtol
#endif
#ifdef F_SysclibForUser_0006
	IMPORT_FUNC  "SysclibForUser",0x7661E728,sprintf
#endif
