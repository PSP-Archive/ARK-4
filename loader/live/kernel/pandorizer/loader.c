#include "main.h"
#include <functions.h>

void (*SysconGetBaryonVersion)(u32*);
u32 (*SysregGetTachyonVersion)(void);
int (*SysconCmdExec)(u8*, int);


u32 eeprom_write(u8 addr, u16 data) {
	int res;
	u8 param[0x60];

	if(addr > 0x7F) 
		return 0x80000102;

	param[0x0C] = 0x73;
	param[0x0D] = 5;
	param[0x0E] = addr;
	param[0x0F] = data;
	param[0x10] = data >> 8;

    res = SysconCmdExec(param, 0);

	if(res < 0) 
		return res;

	return 0;
}

u32 eeprom_read(u8 addr) {
	int res;
	u8 param[0x60];

	if(addr > 0x7F) 
		return 0x80000102;

	param[0x0C] = 0x74;
	param[0x0D] = 3;
	param[0x0E] = addr;

    res = SysconCmdExec(param, 0);

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

	SysconGetBaryonVersion = (void (*)(u32*))FindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x7EC5A957);
	SysregGetTachyonVersion = (u32 (*)(void))FindFunction("sceLowIO_Driver", "sceSysreg_driver", 0xE2A5D1EE);
	SysconCmdExec = (int (*)(u8*, int))FindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x5B9ACC97);

    if (!SysconGetBaryonVersion){
        PRTSTR("Could not find SysconGetBaryonVersion");
        return -1;
    }

    if (!SysregGetTachyonVersion){
        PRTSTR("Could not find SysregGetTachyonVersion");
        return -1;
    }

    if (!SysconCmdExec){
        PRTSTR("Could not find SysconCmdExec");
        return -1;
    }

	SysconGetBaryonVersion(&baryon);
    tachyon = SysregGetTachyonVersion();

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

void loadKernelArk(){
    int battery_type = battery_check();
    switch (battery_type){
        case 0: PRTSTR("Normal -> Pandora"); battery_convert(battery_type); break; // normal -> pandora
        case 1: PRTSTR("Pandora -> Normal"); battery_convert(battery_type); break; // pandora -> normal
        default: PRTSTR("ERROR: this model does not support Pandora batteries"); break;
    }
    k_tbl->KernelDelayThread(3000000);
    int (*_KernelExitVSH)(void*) = FindFunction("sceLoadExec", "LoadExecForKernel", 0x08F7166C);
    _KernelExitVSH(NULL);
}