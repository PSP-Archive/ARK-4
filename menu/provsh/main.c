#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

PSP_MODULE_INFO("PROShell", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(4096);

// Default Start Path
#define START_PATH "ms0:/"

// Current Path
static char cwd[1024];

enum {
	MODE_HOMEBREW = 0,
	MODE_ISO = 1,
	MODE_POPS = 2,
	MODE_MAX = 3,
};

// Runlevel Definition
#define HOMEBREW_RUNLEVEL 0x141
#define ISO_RUNLEVEL 0x123
#define POPS_RUNLEVEL 0x144

// Current Runlevel
// 0x144 = Pops 
// 0x141 = Homebrew
// 0x123 = ISO / CSO / PSN
int mode = MODE_HOMEBREW;

// Copy Move Origin
static char copysource[1024];

// Copy Flags
#define COPY_FOLDER_RECURSIVE 2
#define COPY_DELETE_ON_FINISH 1
#define COPY_KEEP_ON_FINISH 0
#define NOTHING_TO_COPY -1

// Copy Mode
// -1 Nothing
//  0 Copy
//  1 Move
int copymode = NOTHING_TO_COPY;

// Generic Definitions
#define CLEAR 1
#define KEEP 0

// GUI Definitions
#define FONT_SIZE 7
#define PADDING 3
#define FONT_COLOR 0xFFFFFF
#define FONT_SELECT_COLOR 0xFFD800
#define BACK_COLOR 0xBCAD94
#define CENTER_X(s) (240 - ((strlen(s) * FONT_SIZE) >> 1))
#define RIGHT_X(s) (480 - (strlen(s) * FONT_SIZE))
#define CENTER_Y 132
#define FILES_PER_PAGE 18

// Button Test Macro
#define PRESSED(b, a, m) (((b & m) == 0) && ((a & m) == m))

// File Information Structure
typedef struct File
{
	// Next Item
	struct File * next;
	
	// Folder Flag
	int isFolder;
	
	// File Name
	char name[256];
} File;

// Prototypes
void printoob(char * text, int x, int y, unsigned int color);
void updateList(int clearindex);
void paintList(int withclear);
void recursiveFree(File * node);
void start(void);
int delete(void);
int navigate(void);
void copy(int flag);
int paste(void);
File * findindex(int index);
int delete_folder_recursive(char * path);
int copy_file(char * a, char * b);
int copy_folder_recursive(char * a, char * b);
int isPathISO(const char * path);
int isPathCSO(const char * path);
int isGameISO(const char * path);

// Menu Position
int position = 0;

// Number of Files
int filecount = 0;

// File List
File * files = NULL;

u16 ColorConvert(u32 color)
{
	u16 r, g, b;
	r = (u16)((color & 0xFF) >> 3);
	g = (u16)(((color >> 8) & 0xFF) >> 2);
	b = (u16)(((color >> 16) & 0xFF) >> 3);
	return (u16)(r | (g << 5) | (b << 11));
}
__attribute__((noinline)) void PSXFlashScreen(u32 color)
{
	u16 color16 = ColorConvert(color);
	u32 vram = 0x490C0000;
	u32 vramend = vram + 0x96000;
	while(vram < vramend) *(u16*)(vram += 2) = color16;
}

// Entry Point
int main(int argc, char * argv[])
{

	/*
	u32 colors[3] = {0xff, 0xff00, 0xff0000};
	int color = 0;
	
	while (1){
		PSXFlashScreen(colors[color++]);
		if (color >= 3) color = 0;
	}
	*/

	// Set Start Path
	strcpy(cwd, START_PATH);
	
	// Initialize Screen Output
	pspDebugScreenInit();
	
	// Update List
	updateList(CLEAR);
	
	// Paint List
	paintList(KEEP);
	
	// Last Buttons
	unsigned int lastbuttons = 0;
	
	// Input Loop
	while(1)
	{
		// Button Data
		SceCtrlData data;
		
		// Clear Memory
		memset(&data, 0, sizeof(data));
		
		// Read Button Data
		sceCtrlSetSamplingCycle(0);
		sceCtrlSetSamplingMode(1);
		sceCtrlReadBufferPositive(&data, 1);
		
		// Other Commands
		if(filecount > 0)
		{
			// Start File
			if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_START))
			{
				// Start File
				start();
			}
			
			// Delete File
			else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_SELECT))
			{
				// Delete File
				if(delete() == 0)
				{
					// Update List
					updateList(KEEP);
					
					// Paint List
					paintList(CLEAR);
				}
			}
			
			// Position Decrement
			else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_UP))
			{
				// Decrease Position
				if(position > 0) position--;
				
				// Rewind Pointer
				else position = filecount - 1;
				
				// Paint List
				paintList(KEEP);
			}
			
			// Position Increment
			else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_DOWN))
			{
				// Increase Position
				if(position < (filecount - 1)) position++;
				
				// Rewind Pointer
				else position = 0;
				
				// Paint List
				paintList(KEEP);
			}
			
			// Runlevel Change
			else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_LEFT))
			{
				if(--mode < 0) mode = MODE_MAX - 1;
				
				// Paint List
				paintList(CLEAR);
			}
			else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_RIGHT))
			{
				if(++mode >= MODE_MAX) mode = 0;
				
				// Paint List
				paintList(CLEAR);
			}
			
			// Navigate to Folder
			else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_CROSS))
			{
				// Attempt to navigate to Target
				if(navigate() == 0)
				{
					// Update List
					updateList(CLEAR);
					
					// Paint List
					paintList(CLEAR);
				}
			}
			
			// Copy
			else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_SQUARE))
			{
				// Copy File
				copy(COPY_KEEP_ON_FINISH);
				
				// Paint List
				paintList(KEEP);
			}
			
			// Cut
			else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_TRIANGLE))
			{
				// Copy File
				copy(COPY_DELETE_ON_FINISH);
				
				// Paint List
				paintList(KEEP);
			}
			
			// Paste
			else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_CIRCLE))
			{
				// Paste File
				if(paste() == 0)
				{
					// Update List
					updateList(KEEP);
					
					// Paint List
					paintList(CLEAR);
				}
			}
		}
		
		// Copy Buttons to Memory
		lastbuttons = data.Buttons;
		
		// Delay Thread (~100FPS are enough)
		sceKernelDelayThread(10000);
	}
	
	// Kill Main Thread
	sceKernelExitDeleteThread(0);

	// Exit Function
	return 0;
}

// Exit Point
int module_stop(SceSize args, void * argp)
{
	// Return Success
	return 0;
}

// Print Out of Bounds Text
void printoob(char * text, int x, int y, unsigned int color)
{
	// Iterate Characters
	int i = 0; for(; i < strlen(text); i++)
	{
		// Print Character
		pspDebugScreenPutChar(x + i * FONT_SIZE, y, color, (unsigned char)text[i]);
	}
}

// Update File List
void updateList(int clearindex)
{
	// Clear List
	recursiveFree(files);
	files = NULL;
	filecount = 0;
	
	// Open Working Directory
	int directory = sceIoDopen(cwd);
	
	// Opened Directory
	if(directory >= 0)
	{
		// File Info Read Result
		int dreadresult = 1;
		
		// Iterate Files
		while(dreadresult > 0)
		{
			// File Info
			SceIoDirent info;
			
			// Clear Memory
			memset(&info, 0, sizeof(info));
			
			// Read File Data
			dreadresult = sceIoDread(directory, &info);
			
			// Read Success
			if(dreadresult >= 0)
			{
				// Ingore null filename
				if(info.d_name[0] == '\0') continue;

				// Ignore "." in all Directories
				if(strcmp(info.d_name, ".") == 0) continue;
				
				// Ignore ".." in Root Directory
				if(strcmp(cwd, "ms0:/") == 0 && strcmp(info.d_name, "..") == 0) continue;
				
				// Allocate Memory
				File * item = (File *)malloc(sizeof(File));
				
				// Clear Memory
				memset(item, 0, sizeof(File));
				
				// Copy File Name
				strcpy(item->name, info.d_name);
				
				// Set Folder Flag
				item->isFolder = FIO_S_ISDIR(info.d_stat.st_mode);
				
				// New List
				if(files == NULL) files = item;
				
				// Existing List
				else
				{
					// Iterator Variable
					File * list = files;
					
					// Append to List
					while(list->next != NULL) list = list->next;
					
					// Link Item
					list->next = item;
				}
				
				// Increase File Count
				filecount++;
			}
		}
		
		// Close Directory
		sceIoDclose(directory);
	}
	
	// Attempt to keep Index
	if(!clearindex)
	{
		// Fix Position
		if(position >= filecount) position = filecount - 1;
	}
	
	// Reset Position
	else position = 0;
}

char *getModeStr(int mode)
{
	switch(mode)
	{
		case MODE_HOMEBREW:
			return "Application";
		case MODE_ISO:
			return "Game Image";
		case MODE_POPS:
			return "Pops";
	}

	return NULL;
}

// Paint Picker List
void paintList(int withclear)
{
	// Clear Screen
	if(withclear) pspDebugScreenClear();
	
	// Paint Current Path
	printoob(cwd, 10, 10, FONT_COLOR);
	
	// Paint Current Runlevel
	char * strrunlevel = getModeStr(mode);
	printoob(strrunlevel, RIGHT_X(strrunlevel) - 10, 10, FONT_COLOR);
	
	// Paint Serial Number
	char serialnumber[64];
	printoob(serialnumber, 10, 242, FONT_COLOR);
	
	// Paint Controls
	printoob("[ UP & DOWN ]    File Selection", 10, 252, FONT_COLOR);
	printoob("[ LEFT & RIGHT ] Runlevel Selection", 10, 262, FONT_COLOR);
	printoob("[ START ]  Run File", 330, 252, FONT_COLOR);
	printoob("[ SELECT ] Delete", 330, 262, FONT_COLOR);
	printoob("SQUARE    Copy", 345, 120, FONT_COLOR);
	printoob("TRIANGLE  Cut", 345, 130, FONT_COLOR);
	printoob("CIRCLE    Paste", 345, 140, FONT_COLOR);
	printoob("CROSS     Navigate", 345, 150, FONT_COLOR);
	
	// Copy Paste in Progress
	if(copymode != NOTHING_TO_COPY)
	{
		// Get Cache Source Filename
		char * filename = NULL; int i = 0; for(; i < strlen(copysource); i++) if(copysource[i] == '/') filename = copysource + i + 1;
		
		// Paint to Screen
		printoob(filename, 10, 242, FONT_SELECT_COLOR);
	}
	
	// File Iterator Variable
	int i = 0;
	
	// Print Counter
	int printed = 0;
	
	// Paint File List
	File * file = files;
	for(; file != NULL; file = file->next)
	{
		// Printed enough already
		if(printed == FILES_PER_PAGE) break;
		
		// Interesting File
		if(position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE))
		{
			// Default Font Color
			unsigned int color = FONT_COLOR;
			
			// Selected File Font Color
			if(i == position) color = FONT_SELECT_COLOR;
			
			// Print Node Type
			printoob((file->isFolder) ? ("D") : ("F"), 10, 30 + (FONT_SIZE + PADDING) * printed, FONT_COLOR);

			char buf[64];

			strncpy(buf, file->name, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';
			int len = strlen(buf);
			len = 40 - len;

			while(len -- > 0)
			{
				strcat(buf, " ");
			}
			
			// Print Filename
			printoob(buf, 20, 30 + (FONT_SIZE + PADDING) * printed, color);
			
			// Increase Print Counter
			printed++;
		}
		
		// Increase Counter
		i++;
	}
}

// Free Heap Memory
void recursiveFree(File * node)
{
	// End of List
	if(node == NULL) return;
	
	// Nest Further
	recursiveFree(node->next);
	
	// Free Memory
	free(node);
}

// ISO File Check
int isPathISO(const char *path)
{
	const char *ext;

	ext = path + strlen(path) - 4;

	if (ext > path)
	{
		//check extension
		if (stricmp(ext, ".iso") == 0)
		{
			return 1;
		}
	}

	return 0;
}

// CSO File Check
int isPathCSO(const char * path)
{
	// Result (Not CSO)
	int result = 0;
	
	// Open File
	SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	
	// Opened File
	if(fd >= 0)
	{
		// Header Buffer
		unsigned char header[4];
		
		// Read Header
		if(sizeof(header) == sceIoRead(fd, header, sizeof(header)))
		{
			// CSO Header Magic
			unsigned char isoFlags[4] = {
				0x43, 0x49, 0x53, 0x4F
			};
			
			// Valid Magic
			if(0 == memcmp(header, isoFlags, sizeof(header)))
			{
				// CSO File
				result = 1;
			}
		}
		
		// Close File
		sceIoClose(fd);
	}
	
	// Return Result
	return result;
}

// Game ISO File Check (this crashes for some reason...)
int isGameISO(const char * path)
{
	// ISO or CSO File
	if(isPathISO(path) || isPathCSO(path))
	{
		// Game ISO
		return 1;
	}
	
	// Something else...
	return 0;
}

int getRunlevelMode(int mode)
{
	switch(mode)
	{
		case MODE_HOMEBREW:
			return HOMEBREW_RUNLEVEL;
		case MODE_ISO:
			return ISO_RUNLEVEL;
		case MODE_POPS:
			return POPS_RUNLEVEL;
	}

	return -1;
}

// Start Application
void start(void)
{
	// Find File
	File * file = findindex(position);
	
	// Not a valid file
	if(file == NULL || file->isFolder) return;
	
	// Load Execute Parameter
	struct SceKernelLoadExecVSHParam param;
	
	// Clear Memory
	memset(&param, 0, sizeof(param));
	
	// Set Common Parameters
	param.size = sizeof(param);
	
	// Boot Path Buffer
	char bootpath[1024];
	
	// Create Boot Path
	strcpy(bootpath, cwd);
	strcpy(bootpath + strlen(bootpath), file->name);
	sctrlSESetBootConfFileIndex(MODE_INFERNO);
	
	// ISO File Check
	if(isGameISO(bootpath))
	{
		// Correct Runlevel Setting
		mode = MODE_ISO;
	}
	
	// Homebrew Runlevel
	if(mode == MODE_HOMEBREW)
	{
		// Prepare Homebrew Reboot
		param.args = strlen(bootpath) + 1;
		param.argp = bootpath;
		param.key = "game";
	}

	if(mode == MODE_POPS)
	{
		// Prepare Homebrew Reboot
		param.args = strlen(bootpath) + 1;
		param.argp = bootpath;
		param.key = "pops";
	}
	
	// ISO Runlevel
	else if(mode == MODE_ISO)
	{
		// EBOOT Path
		char * ebootpath = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
		
		// Prepare ISO Reboot
		param.args = strlen(ebootpath) + 1;
		param.argp = ebootpath;
		param.key = "umdemu";
		
		// PSN EBOOT Support
		if(strlen(bootpath) >= strlen(START_PATH "EBOOT.PBP") && strcmp(bootpath + strlen(bootpath) - strlen("EBOOT.PBP"), "EBOOT.PBP") == 0)
		{
			// Switch to Galaxy
			sctrlSESetBootConfFileIndex(MODE_NP9660);
			// Disable Galaxy ISO Emulator Patch
			sctrlSESetUmdFile("");
		}
		
		// Normal ISO
		else
		{
			// Enable Galaxy ISO Emulator Patch
			sctrlSESetUmdFile(bootpath);
		}
	}
	
	int runlevel = getRunlevelMode(mode);
	// Trigger Reboot
	sctrlKernelLoadExecVSHWithApitype(runlevel, bootpath, &param);
}

// Delete Application
int delete(void)
{
	// Find File
	File * file = findindex(position);
	
	// Not found
	if(file == NULL) return -1;

	if(strcmp(file->name, "..") == 0) return -2;

	// File Path
	char path[1024];
	
	// Puzzle Path
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);
	
	// Delete Folder
	if(file->isFolder)
	{
		// Add Trailing Slash
		path[strlen(path) + 1] = 0;
		path[strlen(path)] = '/';
		
		// Delete Folder
		return delete_folder_recursive(path);
	}
	
	// Delete File
	else return sceIoRemove(path);
}

// Navigate to Folder
int navigate(void)
{
	// Find File
	File * file = findindex(position);
	
	// Not a Folder
	if(file == NULL || !file->isFolder) return -1;
	
	// Special Case ".."
	if(strcmp(file->name, "..") == 0)
	{
		// Slash Pointer
		char * slash = NULL;
		
		// Find Last '/' in Working Directory
		int i = strlen(cwd) - 2; for(; i >= 0; i--)
		{
			// Slash discovered
			if(cwd[i] == '/')
			{
				// Save Pointer
				slash = cwd + i + 1;
				
				// Stop Search
				break;
			}
		}
		
		// Terminate Working Directory
		slash[0] = 0;
	}
	
	// Normal Folder
	else
	{
		// Append Folder to Working Directory
		strcpy(cwd + strlen(cwd), file->name);
		cwd[strlen(cwd) + 1] = 0;
		cwd[strlen(cwd)] = '/';
	}
	
	// Return Success
	return 0;
}

// Copy File or Folder
void copy(int flag)
{
	// Find File
	File * file = findindex(position);
	
	// Not found
	if(file == NULL) return;
	
	// Copy File Source
	strcpy(copysource, cwd);
	strcpy(copysource + strlen(copysource), file->name);
	
	// Add Recursive Folder Flag
	if(file->isFolder) flag |= COPY_FOLDER_RECURSIVE;
	
	// Set Copy Flags
	copymode = flag;
}

// Paste File or Folder
int paste(void)
{
	// No Copy Source
	if(copymode == NOTHING_TO_COPY) return -1;
	
	// Source and Target Folder are identical
	char * lastslash = NULL; int i = 0; for(; i < strlen(copysource); i++) if(copysource[i] == '/') lastslash = copysource + i;
	char backup = lastslash[1];
	lastslash[1] = 0;
	int identical = strcmp(copysource, cwd) == 0;
	lastslash[1] = backup;
	if(identical) return -2;
	
	// Source Filename
	char * filename = lastslash + 1;
	
	// Required Target Path Buffer Size
	int requiredlength = strlen(cwd) + strlen(filename) + 1;
	
	// Allocate Target Path Buffer
	char * copytarget = (char *)malloc(requiredlength);
	
	// Puzzle Target Path
	strcpy(copytarget, cwd);
	strcpy(copytarget + strlen(copytarget), filename);
	
	// Return Result
	int result = -3;
	
	// Recursive Folder Copy
	if((copymode & COPY_FOLDER_RECURSIVE) == COPY_FOLDER_RECURSIVE)
	{
		// Check Files in current Folder
		File * node = files; for(; node != NULL; node = node->next)
		{
			// Found a file matching the name (folder = ok, file = not)
			if(strcmp(filename, node->name) == 0 && !node->isFolder)
			{
				// Error out
				return -4;
			}
		}
		
		// Copy Folder recursively
		result = copy_folder_recursive(copysource, copytarget);
		
		// Source Delete
		if(result == 0 && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
		{
			// Append Trailing Slash (for recursion to work)
			copysource[strlen(copysource) + 1] = 0;
			copysource[strlen(copysource)] = '/';
			
			// Delete Source
			delete_folder_recursive(copysource);
		}
	}
	
	// Simple File Copy
	else
	{
		// Copy File
		result = copy_file(copysource, copytarget);
		
		// Source Delete
		if(result == 0 && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
		{
			// Delete File
			sceIoRemove(copysource);
		}
	}
	
	// Paste Success
	if(result == 0)
	{
		// Erase Cache Data
		memset(copysource, 0, sizeof(copysource));
		copymode = NOTHING_TO_COPY;
	}
	
	// Free Target Path Buffer
	free(copytarget);
	
	// Return Result
	return result;
}

// Find File Information by Index
File * findindex(int index)
{
	// File Iterator Variable
	int i = 0;
	
	// Find File Item
	File * file = files; for(; file != NULL && i != index; file = file->next) i++;
	
	// Return File
	return file;
}

// Delete Folder (recursively)
int delete_folder_recursive(char * path)
{
	// Internal File List
	File * filelist = NULL;
	
	// Open Working Directory
	int directory = sceIoDopen(path);
	
	// Opened Directory
	if(directory >= 0)
	{
		// File Info Read Result
		int dreadresult = 1;
		
		// Iterate Files
		while(dreadresult > 0)
		{
			// File Info
			SceIoDirent info;
			
			// Clear Memory
			memset(&info, 0, sizeof(info));
			
			// Read File Data
			dreadresult = sceIoDread(directory, &info);
			
			// Read Success
			if(dreadresult >= 0)
			{
				// Valid Filename
				if(strlen(info.d_name) > 0)
				{
					if(strcmp(info.d_name, ".") == 0 || strcmp(info.d_name, "..") == 0)
					{
						continue;
					}
					
					// Allocate Memory
					File * item = (File *)malloc(sizeof(File));
					
					// Clear Memory
					memset(item, 0, sizeof(File));
					
					// Copy File Name
					strcpy(item->name, info.d_name);
					
					// Set Folder Flag
					item->isFolder = FIO_S_ISDIR(info.d_stat.st_mode);
					
					// New List
					if(filelist == NULL) filelist = item;
					
					// Existing List
					else
					{
						// Iterator Variable
						File * list = filelist;
						
						// Append to List
						while(list->next != NULL) list = list->next;
						
						// Link Item
						list->next = item;
					}
				}
			}
		}
		
		// Close Directory
		sceIoDclose(directory);
	}
	
	// List Node
	File * node = filelist;
	
	// Iterate Files
	for(; node != NULL; node = node->next)
	{
		// Directory
		if(node->isFolder)
		{
			// Required Buffer Size
			int size = strlen(path) + strlen(node->name) + 2;
			
			// Allocate Buffer
			char * buffer = (char *)malloc(size);
			
			// Combine Path
			strcpy(buffer, path);
			strcpy(buffer + strlen(buffer), node->name);
			buffer[strlen(buffer) + 1] = 0;
			buffer[strlen(buffer)] = '/';

			// Recursion Delete
			delete_folder_recursive(buffer);
			
			// Free Memory
			free(buffer);
		}
		
		// File
		else
		{
			// Required Buffer Size
			int size = strlen(path) + strlen(node->name) + 1;
			
			// Allocate Buffer
			char * buffer = (char *)malloc(size);
			
			// Combine Path
			strcpy(buffer, path);
			strcpy(buffer + strlen(buffer), node->name);

			// Delete File
			sceIoRemove(buffer);
			
			// Free Memory
			free(buffer);
		}
	}
	
	// Free temporary List
	recursiveFree(filelist);

	// Delete now empty Folder
	return sceIoRmdir(path);
}

// Copy File from A to B
int copy_file(char * a, char * b)
{
	// Chunk Size
	int chunksize = 512 * 1024;
	
	// Reading Buffer
	char * buffer = (char *)malloc(chunksize);
	
	// Accumulated Writing
	int totalwrite = 0;
	
	// Accumulated Reading
	int totalread = 0;
	
	// Result
	int result = 0;
	
	// Open File for Reading
	int in = sceIoOpen(a, PSP_O_RDONLY, 0777);
	
	// Opened File for Reading
	if(in >= 0)
	{
		// Delete Output File (if existing)
		sceIoRemove(b);
		
		// Open File for Writing
		int out = sceIoOpen(b, PSP_O_WRONLY | PSP_O_CREAT, 0777);
		
		// Opened File for Writing
		if(out >= 0)
		{
			// Read Byte Count
			int read = 0;
			
			// Copy Loop (512KB at a time)
			while((read = sceIoRead(in, buffer, chunksize)) > 0)
			{
				// Accumulate Read Data
				totalread += read;
				
				// Write Data
				totalwrite += sceIoWrite(out, buffer, read);
			}
			
			// Close Output File
			sceIoClose(out);
			
			// Insufficient Copy
			if(totalread != totalwrite) result = -3;
		}
		
		// Output Open Error
		else result = -2;
		
		// Close Input File
		sceIoClose(in);
	}
	
	// Input Open Error
	else result = -1;
	
	// Free Memory
	free(buffer);
	
	// Return Result
	return result;
}

// Copy Folder from A to B
int copy_folder_recursive(char * a, char * b)
{
	// Open Working Directory
	int directory = sceIoDopen(a);
	
	// Opened Directory
	if(directory >= 0)
	{
		// Create Output Directory (is allowed to fail, we can merge folders after all)
		sceIoMkdir(b, 0777);
		
		// File Info Read Result
		int dreadresult = 1;
		
		// Iterate Files
		while(dreadresult > 0)
		{
			// File Info
			SceIoDirent info;
			
			// Clear Memory
			memset(&info, 0, sizeof(info));
			
			// Read File Data
			dreadresult = sceIoDread(directory, &info);
			
			// Read Success
			if(dreadresult >= 0)
			{
				// Valid Filename
				if(strlen(info.d_name) > 0)
				{
					// Calculate Buffer Size
					int insize = strlen(a) + strlen(info.d_name) + 2;
					int outsize = strlen(b) + strlen(info.d_name) + 2;
					
					// Allocate Buffer
					char * inbuffer = (char *)malloc(insize);
					char * outbuffer = (char *)malloc(outsize);
					
					// Puzzle Input Path
					strcpy(inbuffer, a);
					inbuffer[strlen(inbuffer) + 1] = 0;
					inbuffer[strlen(inbuffer)] = '/';
					strcpy(inbuffer + strlen(inbuffer), info.d_name);
					
					// Puzzle Output Path
					strcpy(outbuffer, b);
					outbuffer[strlen(outbuffer) + 1] = 0;
					outbuffer[strlen(outbuffer)] = '/';
					strcpy(outbuffer + strlen(outbuffer), info.d_name);
					
					// Another Folder
					if(FIO_S_ISDIR(info.d_stat.st_mode))
					{
						// Copy Folder (via recursion)
						copy_folder_recursive(inbuffer, outbuffer);
					}
					
					// Simple File
					else
					{
						// Copy File
						copy_file(inbuffer, outbuffer);
					}
					
					// Free Buffer
					free(inbuffer);
					free(outbuffer);
				}
			}
		}
		
		// Close Directory
		sceIoDclose(directory);
		
		// Return Success
		return 0;
	}
	
	// Open Error
	else return -1;
}

