#ifndef _BATTERY_H
#define _BATTERY_H


#include <psptypes.h>


u32 eeprom_write(u8 addr, u16 data);
u32 eeprom_read(u8 addr);

int eeprom_error_check(u32 data);

int eeprom_serial_read(u16* pdata);
int eeprom_writeSerial(u16* pdata);

void battery_convert(int battery);
int battery_check(void);


#endif