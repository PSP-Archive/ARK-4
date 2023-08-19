#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

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

int syscon_init(void);

// we don't fully understand how the syscon/baryon version is constructed.
// what we do know is that the 3rd byte (mask 0x00FF0000) is strongly
// correlated with the model
#define BARYON_VERSION_MODEL_CODE(version)  ((version >> 16) & 0xFF)
#define BARYON_MODEL_CODE_IS_HANDSHAKE_TYPE1(code) (code == 0x2C || code == 0x2E || code == 0x40)
#define BARYON_MODEL_CODE_IS_HANDSHAKE_TYPE2(code) (code == 0x30)

unsigned int syscon_get_baryon_version(void);

int syscon_ctrl_led(unsigned int led, unsigned int on);
int syscon_ctrl_hr_power(unsigned int on);
int syscon_handshake_unlock(void);

#ifdef __cplusplus
}
#endif //__cplusplus
