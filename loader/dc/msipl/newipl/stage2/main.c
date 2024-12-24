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

#define REG32(addr) *((volatile uint32_t *)(addr))
#define SYSREG_CLK2_ENABLE_REG	(*(vu32 *)(0xBC100058))

void DcacheClear();
void IcacheClear();

void ClearCaches()
{
	DcacheClear();
	IcacheClear();
}

int entry()
{

	// initialise syscon
	syscon_init();
	syscon_handshake_unlock();
	mspro_init();

	uint32_t baryon_version = syscon_get_baryon_version();
	uint32_t tachyon_version = syscon_get_tachyon_version();

	if (tachyon_version >= 0x600000)
		_sw(0x20070910, 0xbfc00ffc);
	else if (tachyon_version >= 0x400000)
		_sw(0x20050104, 0xbfc00ffc);
	else
		_sw(0x20040420, 0xbfc00ffc);

	char* path = "/TM/DCARK/msipl_11g.bin";
	void* load_addr = 0x40e0000;
	if (tachyon_version <= 0x00400000) {
		path = "/TM/DCARK/msipl_01g.bin";
	} else if (tachyon_version == 0x00500000 || (tachyon_version == 0x00600000 && baryon_version == 0x00243000)) {
		path = "/TM/DCARK/msipl_02g.bin";
	} else if (tachyon_version <= 0x00600000) {
		path = "/TM/DCARK/msipl_03g.bin";
	} else if (tachyon_version == 0x00810000 && baryon_version == 0x002C4000) {
		path = "/TM/DCARK/msipl_04g.bin";
	} else if (tachyon_version <= 0x00800000) {
		path = "/TM/DCARK/msipl_05g.bin";
	} else if (tachyon_version <= 0x00820000 && baryon_version == 0x012E4000) {
		path = "/TM/DCARK/msipl_07g.bin";
	} else if ((tachyon_version == 0x00820000 || tachyon_version == 0x00810000) && baryon_version == 0x002E4000) {
		path = "/TM/DCARK/msipl_09g.bin";
	}

	MsFatMount();

	MsFatOpen(path);

	MsFatRead(load_addr, 0xec000);

	MsFatClose();

	ClearCaches();
	
	return ((int (*)()) load_addr)();
}

