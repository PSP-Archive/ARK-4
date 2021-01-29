#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psploadexec.h>
#include <psputility.h>
#include <string.h>

// Extended Savedata Structures
#include "savedata.h"

// ICON0.PNG Image
#include "icon0.h"

PSP_MODULE_INFO("UNOCrypter", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(0);

// Printf Define
#define printf pspDebugScreenPrintf

// sysmem.prx Imports
int sceKernelSetCompilerVersion(uint32_t version);
int sceKernelSetDevkitVersion(uint32_t version);

// Cheap Workaround if PSPSDK doesn't play along...
// Requires manual hexediting of NIDs in DATA.PSP after compilation.
// #define sceKernelSetCompilerVersion sceKernelGetBlockHeadAddr
// #define sceKernelSetDevkitVersion sceKernelTotalFreeMemSize

// UNO Encryption Key
uint8_t cryptokey[16] = {
	0x10, 0x01, 0x01, 0x10,
	0x00, 0x00, 0x00, 0x10,
	0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

// SECURE.BIN Buffer
uint8_t savedatabuffer[0x100000];

// ICON0.PNG Buffer
uint8_t icon0buffer[0x80];

// PIC1.PNG Buffer
uint8_t pic1buffer[0x100];

// SND0.AT3 Buffer
uint8_t snd0buffer[0x200];

// SECURE.BIN Decrypter
int decrypt(void)
{
	// Prepare Parameter
	SceUtilitySavedataParam660 param;
	memset(&param, 0, sizeof(param));
	
	// 0x00
	param.base.size = sizeof(SceUtilitySavedataParam660);
	
	// 0x04
	param.base.message_lang = 1;
	
	// 0x08
	param.base.ctrl_assign = 1;
	
	// 0x0C
	param.base.main_thread_priority = 0x11;
	
	// 0x10
	param.base.sub_thread_priority = 0x13;
	
	// 0x14
	param.base.font_thread_priority = 0x12;
	
	// 0x18
	param.base.sound_thread_priority = 0x10;
	
	// 0x30
	param.type = SCE_UTILITY_SAVEDATA_TYPE_AUTOLOAD;
	
	// 0x38
	param.overWriteMode = SCE_UTILITY_SAVEDATA_OVERWRITEMODE_ON;
	
	// 0x3C
	strncpy(param.titleId, "NPEH00020", SCE_UTILITY_SD_TITLEID_SIZE);
	
	// 0x4C
	strncpy(param.userId, "DATA00", SCE_UTILITY_SD_USERID_SIZE);
	
	// 0x64
	strncpy(param.fileName, "SECURE.BIN", SCE_UTILITY_SD_FILENAME_SIZE);
	
	// 0x74
	param.pDataBuf = savedatabuffer;
	
	// 0x78
	param.dataBufSize = sizeof(savedatabuffer);
	
	// 0x584
	param.icon0.pDataBuf = icon0buffer;
	param.icon0.dataBufSize = sizeof(icon0buffer);
	param.icon0.dataFileSize = sizeof(icon0buffer);
	
	// 0x5A4
	param.pic1.pDataBuf = pic1buffer;
	param.pic1.dataBufSize = sizeof(pic1buffer);
	param.pic1.dataFileSize = sizeof(pic1buffer);
	
	// 0x5B4
	param.snd0.pDataBuf = snd0buffer;
	param.snd0.dataBufSize = sizeof(snd0buffer);
	param.snd0.dataFileSize = sizeof(snd0buffer);
	
	// 0x5DC (encryption key)
	memcpy(param.secureFileId, cryptokey, SCE_UTILITY_SD_SECUREFILEID_SIZE);
	
	SceUID test = sceIoOpen("ms0:/test.bin", PSP_O_WRONLY | PSP_O_CREAT, 0777);
	if(test >= 0)
	{
		sceIoWrite(test, &param, sizeof(param));
		sceIoClose(test);
	}
	
	// Request Utility Start
	if(sceUtilitySavedataInitStart((SceUtilitySavedataParam *)&param) != 0) return -1;
	
	// Wait for Utility Start
	while(sceUtilitySavedataGetStatus() != 2) sceKernelDelayThread(1000000);
	
	// Decode Savegame
	while(sceUtilitySavedataGetStatus() != 3)
	{
		sceUtilitySavedataUpdate(1);
		sceKernelDelayThread(10000);
	}
	
	// Shutdown Utility
	if(sceUtilitySavedataShutdownStart() != 0) return -2;
	
	// Wait for Shutdown
	while(sceUtilitySavedataGetStatus() != 0) sceKernelDelayThread(1000000);
	
	// Log Result
	printf("Utility Result: %08X\n", param.base.result);
	
	test = sceIoOpen("ms0:/test-end.bin", PSP_O_WRONLY | PSP_O_CREAT, 0777);
	if(test >= 0)
	{
		sceIoWrite(test, &param, sizeof(param));
		sceIoClose(test);
	}
	
	// Open Output File
	SceUID fd = sceIoOpen("ms0:/SECURE.BIN", PSP_O_WRONLY | PSP_O_CREAT, 0777);
	
	// File Open Error
	if(fd < 0) return -3;
	
	// Write SECURE.BIN Data
	int written = sceIoWrite(fd, savedatabuffer, param.dataFileSize);
	
	// Close File
	sceIoClose(fd);
	
	// File Write Error
	if(written < param.dataFileSize) return -4;
	
	// Return Success
	return 0;
}

// SECURE.BIN Encrypter
int encrypt(void)
{
	// Open Input File
	SceUID fd = sceIoOpen("ms0:/SECURE.BIN", PSP_O_RDONLY, 0777);
	
	// File Open Error
	if(fd < 0) return -1;
	
	// Read SECURE.BIN Data
	int read = sceIoRead(fd, savedatabuffer, sizeof(savedatabuffer));
	
	// Close File
	sceIoClose(fd);
	
	// File Read Error
	if(read <= 0) return -2;
	
	// Prepare Parameter
	SceUtilitySavedataParam660 param;
	memset(&param, 0, sizeof(param));
	
	// 0x00
	param.base.size = sizeof(SceUtilitySavedataParam660);
	
	// 0x04
	param.base.message_lang = 1;
	
	// 0x08
	param.base.ctrl_assign = 1;
	
	// 0x0C
	param.base.main_thread_priority = 0x11;
	
	// 0x10
	param.base.sub_thread_priority = 0x13;
	
	// 0x14
	param.base.font_thread_priority = 0x12;
	
	// 0x18
	param.base.sound_thread_priority = 0x10;
	
	// 0x30
	param.type = SCE_UTILITY_SAVEDATA_TYPE_AUTOSAVE;
	
	// 0x38
	param.overWriteMode = SCE_UTILITY_SAVEDATA_OVERWRITEMODE_ON;
	
	// 0x3C
	strncpy(param.titleId, "NPEH00020", SCE_UTILITY_SD_TITLEID_SIZE);
	
	// 0x4C
	strncpy(param.userId, "DATA00", SCE_UTILITY_SD_USERID_SIZE);
	
	// 0x64
	strncpy(param.fileName, "SECURE.BIN", SCE_UTILITY_SD_FILENAME_SIZE);
	
	// 0x74
	param.pDataBuf = savedatabuffer;
	
	// 0x78
	param.dataBufSize = sizeof(savedatabuffer);
	
	// 0x7C
	param.dataFileSize = read;
	
	// 0x80
	strncpy(param.systemFile.title, "UNO\xE2\x84\xA2", SCE_UTILITY_SD_SYSF_TITLE_SIZE);
	
	// 0x100
	strncpy(param.systemFile.savedataTitle, "Game Data", SCE_UTILITY_SD_SYSF_SD_TITLE_SIZE);
	
	// 0x180
	strncpy(param.systemFile.detail, "Settings and Progress", SCE_UTILITY_SD_SYSF_DETAIL_SIZE);
	
	// 0x580
	param.systemFile.parentalLev = 2;
	
	// 0x584
	param.icon0.pDataBuf = icon0;
	param.icon0.dataBufSize = size_icon0;
	param.icon0.dataFileSize = size_icon0;
	
	// 0x5DC (encryption key)
	memcpy(param.secureFileId, cryptokey, SCE_UTILITY_SD_SECUREFILEID_SIZE);
	
	// Request Utility Start
	if(sceUtilitySavedataInitStart((SceUtilitySavedataParam *)&param) != 0) return -1;
	
	// Wait for Utility Start
	while(sceUtilitySavedataGetStatus() != 2) sceKernelDelayThread(1000000);
	
	// Encode Savegame
	while(sceUtilitySavedataGetStatus() != 3)
	{
		sceUtilitySavedataUpdate(1);
		sceKernelDelayThread(10000);
	}
	
	// Shutdown Utility
	if(sceUtilitySavedataShutdownStart() != 0) return -2;
	
	// Wait for Shutdown
	while(sceUtilitySavedataGetStatus() != 0) sceKernelDelayThread(1000000);
	
	// Log Result
	printf("Utility Result: %08X\n", param.base.result);
	
	// Return Success
	return 0;
}

// Minimalistic Button Reader
uint32_t ctrl_read(void)
{
	// Button Structure
	SceCtrlData ctl;
	
	// Read Buttons
	sceCtrlReadBufferPositive(&ctl, 1);
	
	// Return Button Mask
	return ctl.Buttons;
}

// Entry Point
int main(int argc, char * argv[])
{
	// Fake Devkit Data
	sceKernelSetCompilerVersion(0x00030306);
	sceKernelSetDevkitVersion(0x05050010);
	
	// Initialize Debug Output Screen
	pspDebugScreenInit();
	
	// Output Options
	printf("Press X to export SECURE.BIN\n");
	printf("Press O to import SECURE.BIN\n");
	printf("Press Start to exit without doing anything\n");
	
	// Input Wait Loop
	while(1)
	{
		// Read Buttons
		uint32_t buttons = ctrl_read();
		
		// X Button
		if(buttons & PSP_CTRL_CROSS)
		{
			// Decrypt SECURE.BIN
			int result = decrypt();
			
			// Log Result
			printf("Decrypt returned %d\n", result);
			
			// Exit Wait Loop
			break;
		}
		
		// O Button
		else if(buttons & PSP_CTRL_CIRCLE)
		{
			// Encrypt SECURE.BIN
			int result = encrypt();
			
			// Log Result
			printf("Encrypt returned %d\n", result);
			
			// Exit Wait Loop
			break;
		}
		
		// Start Button
		else if(buttons & PSP_CTRL_START)
		{
			// Exit Wait Loop
			break;
		}
	}
	
	// Show Result
	sceKernelDelayThread(5000000);
	
	// Exit to VSH
	sceKernelExitGame();

	// Kill Thread
	sceKernelExitDeleteThread(0);

	// Return Success
	return 0;
}

// Exit Point
int module_stop(SceSize args, void * argp)
{
	// Return Success
	return 0;
}

