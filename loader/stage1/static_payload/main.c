#define SCE_IO_OPEN ? // sceIoOpen
#define SCE_IO_READ ? // sceIoRead
#define SCE_IO_CLOSE ? // sceIoClose
#define CLEAR_CACHE ? // whatever fucking function that works
#define ARK_ENTRY 0x00010000 // don't touch this!
#define ARK_SIZE 0x4000 // by default, we read until EOF or end of scratchpad (16kB)
#define ARK_BIN "ARK.BIN" // ARK payload
#define ARK_PATH "ms0:/PSP/SAVEDATA/ARK_01234/" // you don't really need to change this

// Entry Point
int exploitEntry() __attribute__((section(".text.startup")));
int exploitEntry(){
	int (* IoOpen)(char *, int, int) = (void*)SCE_IO_OPEN;
	int (* IoRead)(int, void *, int) = (void*)SCE_IO_READ;
	int (* IoClose)(int) = (void*)SCE_IO_CLOSE;
	void (* KernelDcacheWritebackAll)(void) = (void*)CLEAR_CACHE;
	void (* arkEntry)(ARKConfig*, FunctionTable*) = (void*)ARK_ENTRY;

	
	int fd = IoOpen(ARK_PATH ARK_BIN, PSP_O_RONLY);
	IoRead(fd, ARK_ENTRY, ARK_SIZE);
	IoClose(fd);
	KernelDcacheWritebackAll();
	arkEntry(ARK_PATH);
}
