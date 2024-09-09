#include "battery.h"

#include "scepaf.h"
#include "kubridge.h"
#include <systemctrl.h>


void (*SysconGetBaryonVersion)(u32*);
u32 (*SysregGetTachyonVersion)(void);
int (*SysconCmdExec)(u8*, int);


u32 eeprom_write(u8 addr, u16 data) {
	int res;
	u8 param[0x60];
	struct KernelCallArg args;

	if(addr > 0x7F) 
		return 0x80000102;

	param[0x0C] = 0x73;
	param[0x0D] = 5;
	param[0x0E] = addr;
	param[0x0F] = data;
	param[0x10] = data >> 8;

	scePaf_memset(&args, 0, sizeof(args));
	args.arg1 = (int)param;
	kuKernelCall(SysconCmdExec, &args);
	res = args.ret1;

	if(res < 0) 
		return res;

	return 0;
}

u32 eeprom_read(u8 addr) {
	int res;
	u8 param[0x60];
	struct KernelCallArg args;

	if(addr > 0x7F) 
		return 0x80000102;

	param[0x0C] = 0x74;
	param[0x0D] = 3;
	param[0x0E] = addr;

	scePaf_memset(&args, 0, sizeof(args));
	args.arg1 = (int)param;
	kuKernelCall(SysconCmdExec, &args);
	res = args.ret1;

	if(res < 0) 
		return res;

	return (param[0x21] << 8) | param[0x20];
}

int eeprom_error_check(u32 data) {
	if ((data & 0x80250000) == 0x80250000) 
		return -1;
	else if (data & 0xFFFF0000) 
		return ((data & 0xFFFF0000) >> 16);
	return 0;
}

int eeprom_serial_read(u16* pdata) {
	u32 data = eeprom_read(0x07);
	int err = eeprom_error_check(data);
	if(!(err < 0)) {
		pdata[0] = (data & 0xFFFF);
		data = eeprom_read(0x09);
		err = eeprom_error_check(data);
		if (!(err < 0)) 
			pdata[1] = (data & 0xFFFF);
		else 
			err = data;
	}
	else 
		err = data;

	return err;
}

int eeprom_writeSerial(u16* pdata) {
	int err = eeprom_write(0x07, pdata[0]);
	if(!err) 
		err = eeprom_write(0x09, pdata[1]);

	return err;
}



void battery_convert(int battery) {
	u16 buffer[2];
	
	if (battery < 0 || battery > 1) 
		return;

	if (battery) {
		buffer[0] = 0x1234;
		buffer[1] = 0x5678;
	} else {
		buffer[0] = 0xFFFF;
		buffer[1] = 0xFFFF;
	}
	eeprom_writeSerial(buffer);
}

int battery_check(void) {
	int is_pandora;
	u32 baryon, tachyon;
	struct KernelCallArg args;

	SysconGetBaryonVersion = (void (*)(u32*))sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x7EC5A957);
	SysregGetTachyonVersion = (u32 (*)(void))sctrlHENFindFunction("sceLowIO_Driver", "sceSysreg_driver", 0xE2A5D1EE);
	SysconCmdExec = (int (*)(u8*, int))sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x5B9ACC97);

	scePaf_memset(&args, 0, sizeof(args));
	args.arg1 = (int)&baryon;
	kuKernelCall(SysconGetBaryonVersion, &args);

	scePaf_memset(&args, 0, sizeof(args));
	kuKernelCall(SysregGetTachyonVersion, &args);
	tachyon = args.ret1;

	if (tachyon >= 0x00500000 && baryon >= 0x00234000) 
		is_pandora = -1;
	else {   
		u16 serial[2];
		eeprom_serial_read(serial);
	
		if(serial[0] == 0xFFFF && serial[1] == 0xFFFF) 
			is_pandora = 1;
		else 
			is_pandora = 0;
	}
	return is_pandora;
}