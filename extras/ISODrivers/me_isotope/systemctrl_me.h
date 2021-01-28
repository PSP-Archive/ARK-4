#ifndef __SYSTEMCTRL_ME_H__
#define __SYSTEMCTRL_ME_H__


#include <pspsdk.h>
#include <pspkernel.h>
#include <psploadexec_kernel.h>
#include <pspinit.h>
#include <psploadcore.h>

enum BootLoadFlags
{
	BOOTLOAD_VSH = 1,
	BOOTLOAD_GAME = 2,
	BOOTLOAD_UPDATER = 4,
	BOOTLOAD_POPS = 8,
	BOOTLOAD_UMDEMU = 64, /* for original NP9660 */
};
enum SEUmdModes
{
	MODE_UMD = 0,
	MODE_OE_LEGACY = 1,
	MODE_MARCH33 = 2,
	MODE_NP9660 = 3,
};

enum MEUmdModes
{
	ME_MODE_UMD = 0,
	ME_MODE_OE_LEGACY = 1,
	ME_MODE_MARCH33 = 2,
	ME_MODE_NP9660 = 3,
	ME_MODE_ME = 4,
};

typedef struct
{
	int magic; /*  */
	int hidecorrupt;
	int	skiplogo;
	int umdactivatedplaincheck;//Hide mac

	int gamekernel150;
	int skipgameboot;//executebootbinGameboot skip
	int startupprog;//UmdVideoPatch
	int umdmode;

	int execute_pboot;
	int	vshcpuspeed;
	int	vshbusspeed; 
	int	umdisocpuspeed; 

	int	umdisobusspeed; 
	int fakeregion;
	int executeopnssmp;//freeumdregionexecute OPNSSMP
	int	usbprotect;

	int usbdevice;
	int novshmenu;
	int usbcharge;
	int netupdate;

	int hidepng;
	int plugvsh;
	int pluggame;
	int plugpop;
	
	int versiontxt;
	int fastms;//
	int reserved[3];
} SEConfig;

#define MINI_MAGIC 0x3031454D

typedef struct{
	u32 magic;
	int vsh_color;
	int recovery_color;
}VshConfig;

int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param);
int sctrlKernelLoadExecVSHDisc(const char *file, struct SceKernelLoadExecVSHParam *param);
int sctrlKernelLoadExecVSHDiscUpdater(const char *file, struct SceKernelLoadExecVSHParam *param);
int sctrlKernelLoadExecVSHMs1(const char *file, struct SceKernelLoadExecVSHParam *param);
int sctrlKernelLoadExecVSHMs2(const char *file, struct SceKernelLoadExecVSHParam *param);
int sctrlKernelLoadExecVSHMs3(const char *file, struct SceKernelLoadExecVSHParam *param);
int sctrlKernelLoadExecVSHMs4(const char *file, struct SceKernelLoadExecVSHParam *param);
int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param);
int sctrlKernelSetInitApitype(int apitype);
int sctrlKernelSetInitFileName(char *filename);
int sctrlKernelSetInitKeyConfig(int key);
int sctrlKernelSetUserLevel(int level);
int sctrlKernelSetDevkitVersion(int version);
void* sctrlKernelGetPartition(int pid);

int sctrlKernelBootFrom();
int sctrlKernelMsIsEf();

int	sctrlHENIsSE();
int	sctrlHENIsDevhook();
int sctrlHENGetVersion();

u32 sctrlHENFindFunction(const char* szMod, const char* szLib, u32 nid);
#define FindProc sctrlHENFindFunction

typedef int (* STMOD_HANDLER)(SceModule2 *);
STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler);

int sctrlHENSetMemory(u32 p2, u32 p8);
void sctrlHENLoadModuleOnReboot(char *module_after, void *buf, int size, int flags);

#define PatchSyscall sctrlHENPatchSyscall
void sctrlHENPatchSyscall(u32 addr, void *newaddr);

PspIoDrv *sctrlHENFindDriver(char *drvname);

int sctrlSEGetVersion();
int sctrlSEGetConfig(SEConfig *config);
int sctrlSESetConfig(SEConfig *config);
int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs);
int sctrlSEUmountUmd(void);
void sctrlSESetDiscOut(int out);
void sctrlSESetDiscType(int type);
int  sctrlSEGetDiscType(void);

char *sctrlSEGetUmdFile();
void sctrlSESetUmdFile(const char *umd);
char *sctrlSEGetUmdFileEx(char *input);
void sctrlSESetUmdFileEx(const char *umd, char *input);

void sctrlSEApplyConfig(SEConfig *config);
void sctrlSESetBootConfFileIndex(int no);

void *sctrlKernelMalloc(size_t size);
int sctrlKernelFree(void *p);

u32 sctrlKernelResolveNid(const char *szLib, u32 nid);

void SetSpeed(int cpu, int bus);

#endif