#include <pspsdk.h>
#include <string.h>
#include <sysreg.h>
#include <syscon.h>
#include <gpio.h>
#include <fat.h>

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a)
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a)

void DcacheClear();
void IcacheClear();

void ClearCaches()
{
	DcacheClear();
	IcacheClear();
}

uint32_t GetTachyonVersion()
{
	uint32_t ver = _lw(0xbc100040);
	
	if (ver & 0xFF000000)
		return (ver >> 8);

	return 0x100000;
}

int entry(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2)
{

	SYSREG_CLK2_ENABLE_REG |= 0x02;
	REG32(0xbc10007c) |= 0xc8;

	sysreg_io_enable_gpio();

	// initialise syscon
	syscon_init();
	syscon_handshake_unlock();
	mspro_init();

	uint32_t baryon_version = syscon_get_baryon_version();
	uint32_t tachyon_version = GetTachyonVersion();

	if (tachyon_version >= 0x600000)
		_sw(0x20070910, 0xbfc00ffc);
	else if (tachyon_version >= 0x400000)
		_sw(0x20050104, 0xbfc00ffc);
	else
		_sw(0x20040420, 0xbfc00ffc);

	char* path = "/TM/DCARK/ipl_11g.bin";
	void* load_addr = 0x40e0000;
	if (tachyon_version <= 0x400000) {
		path = "/TM/DCARK/ipl_01g.bin";
	} else if (tachyon_version == 0x500000 || (tachyon_version == 0x600000 && baryon_version == 0x243000)) {
		path = "/TM/DCARK/ipl_02g.bin";
	} else if (tachyon_version <= 0x600000) {
		path = "/TM/DCARK/ipl_03g.bin";
	} else if (tachyon_version == 0x810000 && baryon_version == 0x2C4000) {
		path = "/TM/DCARK/ipl_04g.bin";
	} else if (tachyon_version <= 0x800000) {
		path = "/TM/DCARK/ipl_05g.bin";
	} else if (tachyon_version == 0x810000 && baryon_version == 0x2E4000) {
		path = "/TM/DCARK/ipl_07g.bin";
	} else if (tachyon_version == 0x820000 && baryon_version == 0x2E4000) {
		path = "/TM/DCARK/ipl_09g.bin";
	}

	MsFatMount();

	MsFatOpen(path);

	MsFatRead(load_addr, 0xC000+0xe0000);

	MsFatClose();

	ClearCaches();

	syscon_ctrl_led(0, 1);
	syscon_ctrl_led(1, 1);
	
	return ((int (*)()) load_addr)();
}

