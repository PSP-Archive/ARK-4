#include <pspsdk.h>
#include <macros.h>

#define REG32(addr)                 *((volatile uint32_t *)(addr))

int (* sceReboot)(void *a0, void *a1, void *a2, void *a3) = (void *)0x88c00000;
int (* sceRebootDacheWritebackInvalidateAll)(void) = (void *)0x88c03b80;
int (* sceRebootIcacheInvalidateAll)(void) = (void *)0x88c03f50;
int (* sceBootLfatOpen)(char *file) = (void *)0x88c07618;

// Invalidate Instruction and Data Cache
void flushCache(void)
{
    // Invalidate Data Cache
    sceRebootDacheWritebackInvalidateAll();
    // Invalidate Instruction Cache
    sceRebootIcacheInvalidateAll();
}

char path[128];

int _sceBootLfatMount()
{
    return MsFatMount();
}

int _sceBootLfatOpen(char * filename)
{
    if(strcmp(filename, "/kd/lfatfs.prx") == 0) {
        strcpy(filename, "/tmctrl150.prx");
    }

    strcpy(path, "/TM/DCARK/150");
	strcat(path, filename);

	return MsFatOpen(path);
}

int _sceBootLfatRead(char * buffer, int length)
{
    return MsFatRead(buffer, length);
}

int _sceBootLfatClose(void)
{
    return MsFatClose();
}

int loadCoreModuleStartPatched(void *a0, void *a1, int (* module_start)(void *, void *))
{
	/* No Plain Module Check Patch */
	_sw(0x340D0001, 0x880152e0);
	flushCache();

	return module_start(a0, a1);
}

int _arkReboot(void *a0, void *a1, void *a2, void *a3)
{
	memcpy((void *)0x883f0000, (void *)0xbfc00200, 0x200);

	MAKE_CALL(0x88c00074, _sceBootLfatMount);
	MAKE_CALL(0x88c00084, _sceBootLfatOpen);
    MAKE_CALL(0x88c000b4, _sceBootLfatRead);
    MAKE_CALL(0x88c000e0, _sceBootLfatClose);
	
	// Patch the call to LoadCore module_start
	_sw(0x02403021, 0x88c00fec);
	MAKE_CALL(0x88c00ff8, loadCoreModuleStartPatched);

    // Patch removeByDebugSection, make it return 1	
	_sw(0x03e00008, 0x88c01d20);
	_sw(0x24020001, 0x88c01d24);
	
	flushCache();

    // GPIO enable
	REG32(0xbc10007c) |= 0xc8;
	__asm("sync"::);
	
	syscon_init();
    
    syscon_ctrl_ms_power(1);

	return sceReboot(a0, a1, a2, a3);	
}
