#ifndef __SYSREG_H__
#define __SYSREG_H__

#include <psptypes.h>

// 0xBC10003C sceSysregGenerateVblankIRQ sceSysregVblankIRQPolar sceSysregDoTimerEvent
// 0xBC100040 sceSysregGetTachyonVersion sceSysregSetAwEdramSize
#define SYSREG_RESET_ENABLE_REG	(*(vu32 *)(0xBC10004C))
#define SYSREG_BUSCLK_ENABLE_REG	(*(vu32 *)(0xBC100050))
#define SYSREG_CLK1_ENABLE_REG	(*(vu32 *)(0xBC100054))
#define SYSREG_CLK2_ENABLE_REG	(*(vu32 *)(0xBC100058))
// 0xBC10005C sceSysregAtahddClkSelect sceSysregMsifClkSelect
// 0xBC100060 sceSysregAudioClkoutClkSelect sceSysregLcdcClkSelect sceSysregAudioClkSelect sceSysregApbTimerClkSelect
// 0xBC100064 sceSysregSpiClkSelect
// 0xBC100068 sceSysregPllSetOutSelect sceSysregPllUpdateFrequency
#define SYSREG_UNK_IO_ENABLE_REG	(*(vu32 *)(0xBC100074))
#define SYSREG_IO_ENABLE_REG		(*(vu32 *)(0xBC100078))
// 0xBC100090 sceSysregGetFuseId
// 0xBC100098 sceSysregGetFuseConfig
// 0xBC1000FD sceSysregPllSetOutSelect

u32 sceSysregSpiClkSelect(int a1,int a2);
u32 sceSysregSpiClkEnable(u32 bit);

void SysregReset(u32 mask, u32 enable);
void SysregBusclk(u32 mask, u32 enable);

void SysregResetKirkEnable();
void SysregBusclkKirkEnable();
void SysregBusclkKirkDisable();
void SysregResetKirkDisable();

#endif
