#ifndef __SYSCON_H__
#define __SYSCON_H__

/*
	CMD 07,08 : CTRL data

	all bits are active LOW

	+03-06:buttons (cmd 07/08)
	+07   :Analog  (cmd 08)
	+08   :Analog  (cmd 08)
*/

#define REG32(ADDR) (*(vu32*)(ADDR))

#define SYSCON_CTRL_ALLOW_UP  0x00000001
#define SYSCON_CTRL_ALLOW_RT  0x00000002
#define SYSCON_CTRL_ALLOW_DN  0x00000004
#define SYSCON_CTRL_ALLOW_LT  0x00000008
#define SYSCON_CTRL_TRIANGLE  0x00000010
#define SYSCON_CTRL_CIRCLE    0x00000020
#define SYSCON_CTRL_CROSS     0x00000040
#define SYSCON_CTRL_RECTANGLE 0x00000080
#define SYSCON_CTRL_SELECT    0x00000100
#define SYSCON_CTRL_LTRG      0x00000200
#define SYSCON_CTRL_RTRG      0x00000400
#define SYSCON_CTRL_START     0x00000800
#define SYSCON_CTRL_HOME      0x00001000
#define SYSCON_CTRL_HOLD      0x00002000
#define SYSCON_CTRL_WLAN      0x00004000
#define SYSCON_CTRL_HPR_EJ    0x00008000
#define SYSCON_CTRL_VOL_UP    0x00010000
#define SYSCON_CTRL_VOL_SN    0x00020000
#define SYSCON_CTRL_LCD       0x00040000
#define SYSCON_CTRL_NOTE      0x00080000
#define SYSCON_CTRL_UMD_EJCT  0x00100000
#define SYSCON_CTRL_UNKNOWN   0x00200000 /* is not-service mode ? */

/*
	receive +00 : generic status

  bit 0 : AC power input
  bit 1 : sceSysconSetWlanPowerCallback
  bit 2 : sceSyscon_driver_Unkonow_805180d1
  bit 3 : sceSysconSetAlarmCallback
  bit 4 : power switch on
  bit 5 : sceSysconSetLowBatteryCallback
  bit 6 : ?
  bit 7 : ?
*/
#define SYSCON_STS_AC_ACTIVE   0x01
#define SYSCON_STS_WLAN_POW    0x02
#define SYSCON_STS_ALARM       0x08
#define SYSCON_STS_POWRE_SW_ON 0x10
#define SYSCON_STS_LOW_BATTERY 0x20

/*
	receive +02 : response code

	00-7f : response of COMMAND 00-7f
	80-8f : done / error code
*/

// 1.Receive 1st packet of twice command , send same command again
// 2.CHECK SUM error
#define SYSCON_RES_80 0x80

// SYSCON BUSY , send same command again
#define SYSCON_RES_81 0x81

// Done 2nd packet of twice command
#define SYSCON_RES_OK 0x82

// parameter size error
#define SYSCON_RES_83 0x83

// twice send command error
#define SYSCON_RES_86 0x86

int sceSysconInit(void);
int Syscon_cmd(u8 *tx_buf,u8 *rx_buf);

u32 Syscon_wait(u32 usec);

int sceSysconCommonWrite(u32 param,u8 cmd,u8 tx_len);
int sceSysconCommonRead(u32 *param,u8 cmd);

/*
SYSCON COMMAND
command code : write size (within cmd and ws) : read size  (without sts,len):
*/

static inline int sceSysconNop(void){ return sceSysconCommonWrite(0,0x00,2); }								// 00 : 2 : 3
static inline int sceSysconGetBaryonVersion(u32 *ptr){ return sceSysconCommonRead(ptr,0x01); }	// 01 : 2 : 7
static inline int sceSysconGetDigitalKey(u32 *ptr){ return sceSysconCommonRead(ptr,0x02); }	// 02 : 2 : 5
static inline int pspSyscon_driver_Unkonow_3b657a27(u32 *ptr){ return sceSysconCommonRead(ptr,0x05); }	// 05 : 2 : 7
//06 :  ? :  ? : (ctrl read?)
static inline int pspSysconGetCtrl1(u32 *ctrl){ return sceSysconCommonRead(ctrl,0x07); }				// 07 : 2 : 7
              int pspSysconGetCtrl2(u32 *ctrl,u8 *vol1,u8 *vol2);									// 08 : 2 : 9 (ctrl read w. analogStick)
              
static inline int sceSysconReadClock(u32 *ptr){ return sceSysconCommonRead(ptr,0x09); }					// 09 : 2 : 7
static inline int sceSysconReadAlarm(u32 *ptr){ return sceSysconCommonRead(ptr,0x0a); }				// 0A : 2 : 7
static inline int sceSysconGetPowerSupplyStatus(u32 *ptr){ return sceSysconCommonRead(ptr,0x0b); }// 0B : 2 : 7
static inline int sceSysconGetTachyonWDTStatus(u32 *ptr){ return sceSysconCommonRead(ptr,0x0c); }// 0C : 2 : 7
static inline int sceSysconGetBattVolt(u32 *ptr){ return sceSysconCommonRead(ptr,0x0d); }
// 0D
static inline int sceSysconGetWakeUpFactor(u32 *ptr){ return sceSysconCommonRead(ptr,0x0e); }// 0E : 2 : 7
static inline int sceSysconGetWakeUpReq(u32 *ptr){ return sceSysconCommonRead(ptr,0x0f); }// 0F : 2 : 7
// 10 - sceSysconInitPowerStatus
static inline int sceSysconGetTimeStamp(u32 *ptr){ return sceSysconCommonRead(ptr,0x11); }// 11 : 2 : ?
static inline int sceSysconGetVideoCable(u32 *ptr){ return sceSysconCommonRead(ptr, 0x12); }
static inline int pspSysconMsOn(void) { return sceSysconCommonWrite(0,0x1c,2); }							// 1C : 2 : 3
static inline int pspSysconMsOff(void) { return sceSysconCommonWrite(0,0x1d,2); }							// 1D : 2 : ?
static inline int pspSysconWlanOn(void) { return sceSysconCommonWrite(0,0x1e,2); }							// 1E : 2 : ?
static inline int pspSysconWlanOff(void) { return sceSysconCommonWrite(0,0x1f,2); }						// 1F : 2 : ?
static inline int sceSysconWriteClock(u32 val){ return sceSysconCommonWrite(val,0x20,6); }				// 20 : 6 : 3
// 21: sceSysconSetUSBStatus
static inline int sceSysconWriteAlarm(u32 val){ return sceSysconCommonWrite(val,0x22,6); }				// 22 : 6 : 3
//23 :  ? :  ? : sceSysconWriteScratchPad (RTC)
//24 :  3 ;  ? : sceSysconReadScratchPad (RTC)
//25 :  a :  0 : pspSysconSendSetParam
//26 :  2 :  ? : pspSysconReceiveSetParam
//27-30
static inline int sceSysconCtrlTachyonWDT(u32 val){ return sceSysconCommonWrite(val,0x31,3); }
static inline int sceSysconResetDevice(u8 val){ return sceSysconCommonWrite(val,0x32,3); }	// 32 : 3 : 3
static inline int sceSysconCtrlAnalogXYPolling(u8 sw){ return sceSysconCommonWrite(sw,0x33,3); }	// 33 : 3 : 3
static inline int sceSysconCtrlHRPower(u8 sw){ return sceSysconCommonWrite(sw,0x34,3); }		// 34 : 3 : 3 
static inline void pspSysconPowerStandby(void){ sceSysconCommonWrite(0,0x35,2); }				// 35 : 2 : 3
static inline void pspSysconPowerSuspend(void){ sceSysconCommonWrite(0,0x36,2); }				// 36 : 2 : 3
static inline void sceSysconGetBattVoltAD(u32 *ptr){ sceSysconCommonRead(ptr, 0x37); }

static inline int sceSysconGetPommelVersion(u32 *ptr){ return sceSysconCommonRead(ptr,0x40); }	// 40 : 2 : 4-7
static inline int sceSysconGetPolestarVersion(u32 *ptr){ return sceSysconCommonRead(ptr,0x41); }	// 41 : 2 : 4-7
static inline int sceSysconCtrlVoltage(u8 idx, u8 val){ return sceSysconCommonWrite(val << 8 | idx,0x42,5); }		// 42 : 5 : 3
static inline int sceSysconCtrlPower(u32 val){ return sceSysconCommonWrite(val,0x45,5); }		// 45 : 5 : 3
static inline int sceSysconGetPowerStatus(u32 *sts){ return sceSysconCommonRead(sts,0x46); }	// 46 : 2 : ?
              int sceSysconCtrlLED(int sel,int is_on);
static inline int sceSysconWritePommelReg(u32 val){ return sceSysconCommonWrite(val,0x48,5); }// 48 :  5 :  3
static inline int sceSysconReadPommelReg(u32 val){ return sceSysconCommonWrite(val,0x49,3); }// 49 :  3 :  2
static inline int sceSysconCtrlNandPower(u32 *ptr){ return sceSysconCommonRead(ptr,0x4a); }		// 4a : 2 : 7
static inline int sceSysconCtrlLeptonPower(u8 sw){ return sceSysconCommonWrite(sw,0x4b,3); }
static inline int sceSysconCtrlMsPower(u8 sw){ return sceSysconCommonWrite(sw,0x4c,3); }			// 4C : 3 : 3
static inline int sceSysconCtrlWlanPower(u32 val){ return sceSysconCommonWrite(val,0x4d,3); }	// 4d : 3 : 3

// pspSyscon_driver_Unkonow_806d4d6c
// pspSyscon_driver_Unkonow_eab13fbe
// pspSyscon_driver_Unkonow_c5075828
static inline int sceSysconWritePolestarReg(u32 val){ return sceSysconCommonWrite(val,0x4e,5); }			// 4e : 5 : 3
//4f :  3 :  5 : sceSysconReadPolestarReg
// pspSyscon_driver_Unkonow_d8471760
// pspSyscon_driver_Unkonow_eab13fbe
// pspSyscon_driver_Unkonow_c5075828

static inline int sceSysconCtrlDvePower(u32 val){ return sceSysconCommonWrite(val,0x52,3); }
static inline int sceSysconCtrlBtPower(u32 val){ return sceSysconCommonWrite(val,0x53,3); }
static inline int sceSysconCtrlUsbPower(u32 val){ return sceSysconCommonWrite(val,0x55,3); }
static inline int sceSysconCtrlCharge(u32 val){ return sceSysconCommonWrite(val,0x56,3); }
static inline int sceSysconBatteryNop(u32 val){ return sceSysconCommonWrite(0,0x60,2); }			// 60 : 2 : 3
static inline int sceSysconBatteryGetStatusCap(u32 *ptr){ return sceSysconCommonRead(ptr,0x61); }// 61 : 2 : 6
static inline int sceSysconBatteryGetTemp(u32 *ptr){ return sceSysconCommonRead(ptr,0x62); }	// 62 : 2 : 4-7
static inline int sceSysconBatteryGetVolt(u32 *ptr){ return sceSysconCommonRead(ptr,0x63); }	// 63 : 2 : 4-7
static inline int sceSysconBatteryGetElec(u32 *ptr){ return sceSysconCommonRead(ptr,0x64); }	// 64 : 2 : 4-7
static inline int sceSysconBatteryGetRCap(u32 *ptr){ return sceSysconCommonRead(ptr,0x65); }// 65 : 2 : 4-7
static inline int sceSysconBatteryGetCap(u32 *ptr){ return sceSysconCommonRead(ptr,0x66); }// 66 : 2 : 4-7
static inline int sceSysconBatteryGetFullCap(u32 *ptr){ return sceSysconCommonRead(ptr,0x67); }// 67 : 2 : 4-7
static inline int sceSysconBatteryGetIFC(u32 *ptr){ return sceSysconCommonRead(ptr,0x68); }// 68 : 2 : 4-7
static inline int sceSysconBatteryGetLimitTime(u32 *ptr){ return sceSysconCommonRead(ptr,0x69); }// 69 : 2 : 4-7

static inline int sceSysconBatteryGetStatus(u32 *ptr){ return sceSysconCommonRead(ptr,0x6a);}	// 6a : 2 : 4-7
static inline int sceSysconBatteryGetCycle(u32 *ptr){ return sceSysconCommonRead(ptr,0x6b); }	// 6b : 2 : 4-7
static inline int sceSysconBatteryGetSerial(u32 *ptr){ return sceSysconCommonRead(ptr,0x6c); }// 6c : 2 : 4-7
// 6d :  2 :  8 : sceSysconBatteryGetInfo
static inline int sceSysconBatteryGetTempAD(u32 *ptr){ return sceSysconCommonRead(ptr,0x6e);}	// 6e : 2 : 4-7
static inline int sceSysconBatteryGetVoltAD(u32 *ptr){ return sceSysconCommonRead(ptr,0x6f);}	// 6f : 2 : 4-7
static inline int sceSysconBatteryGetElecAD(u32 *ptr){ return sceSysconCommonRead(ptr,0x70);}	// 70 : 2 : 4-7
static inline int sceSysconBatteryGetTotalElec(u32 *ptr){ return sceSysconCommonRead(ptr,0x71);}	// 71 : 2 : 4-7
static inline int sceSysconBatteryGetChargeTime(u32 *ptr){ return sceSysconCommonRead(ptr,0x72);}	// 72 : 2 : 4-7
// 73 :  5 :  3 : pspSyscon_driver_Unkonow_1165c864
// 74 :  3 :  6?: pspSyscon_driver_Unkonow_68ef0bef
// 7f :  3 :  4 : pspSysconBatteryAuth

// int seed_gen1(u8 *random_key, u8 *random_key_dec_resp_dec);
// int seed_gen2(u8 *rand_xor, u8 *key_86, u8 *random_key, u8 *random_key_dec_resp_dec);

#endif
