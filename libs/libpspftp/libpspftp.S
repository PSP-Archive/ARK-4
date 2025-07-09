	.set noreorder

#include "pspimport.s"

// Build files
// libpspftp_XXXX.o

#ifdef F_libpspftp_0000
	IMPORT_START "libpspftp",0x40090000
#endif
#ifdef F_libpspftp_0001
	IMPORT_FUNC  "libpspftp",0xABB2CEA8,ftpdSetMsgHandler
#endif
#ifdef F_libpspftp_0002
	IMPORT_FUNC  "libpspftp",0xF879ADEE,ftpdGetDevice
#endif
#ifdef F_libpspftp_0003
	IMPORT_FUNC  "libpspftp",0xFB54A5ED,ftpdSetDevice
#endif
#ifdef F_libpspftp_0004
	IMPORT_FUNC  "libpspftp",0xDF797381,ftpdExitHandler
#endif
#ifdef F_libpspftp_0005
	IMPORT_FUNC  "libpspftp",0x9384FEB9,ftpdLoop
#endif
#ifdef F_libpspftp_0006
	IMPORT_FUNC  "libpspftp",0x3F5DA38D,ftpInit
#endif
#ifdef F_libpspftp_0007
	IMPORT_FUNC  "libpspftp",0x43556842,ftpClean
#endif
#ifdef F_libpspftp_0008
	IMPORT_FUNC  "libpspftp",0x37F497EB,ftpConnect
#endif
#ifdef F_libpspftp_0009
	IMPORT_FUNC  "libpspftp",0xE0A34D9A,ftpLogin
#endif
#ifdef F_libpspftp_0010
	IMPORT_FUNC  "libpspftp",0xBBB22CB4,ftpDisconnect
#endif
#ifdef F_libpspftp_0011
	IMPORT_FUNC  "libpspftp",0x930386EA,ftpCWD
#endif
#ifdef F_libpspftp_0012
	IMPORT_FUNC  "libpspftp",0x5AD8C8EC,ftpLIST
#endif
#ifdef F_libpspftp_0013
	IMPORT_FUNC  "libpspftp",0x27FDE222,ftpDELE
#endif
#ifdef F_libpspftp_0014
	IMPORT_FUNC  "libpspftp",0x5C214B09,ftpRMD
#endif
#ifdef F_libpspftp_0015
	IMPORT_FUNC  "libpspftp",0x6F08D180,ftpMKD
#endif
#ifdef F_libpspftp_0016
	IMPORT_FUNC  "libpspftp",0x37AA6C8E,ftpAPPE
#endif
#ifdef F_libpspftp_0017
	IMPORT_FUNC  "libpspftp",0x7F2D2871,ftpSTOR
#endif
#ifdef F_libpspftp_0018
	IMPORT_FUNC  "libpspftp",0x7C7BAEBD,ftpRETR
#endif
