#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

int syscon_init(void);

// we don't fully understand how the syscon/baryon version is constructed.
// what we do know is that the 3rd byte (mask 0x00FF0000) is strongly
// correlated with the model
#define BARYON_VERSION_MODEL_CODE(version)  ((version >> 16) & 0xFF)
#define BARYON_MODEL_CODE_IS_HANDSHAKE_TYPE1(code) (code == 0x2C || code == 0x2E || code == 0x40)
#define BARYON_MODEL_CODE_IS_HANDSHAKE_TYPE2(code) (code == 0x30)

unsigned int syscon_get_baryon_version(void);

int syscon_ctrl_power(unsigned int dev, unsigned int arg);

typedef enum {
    SYSCON_LED_MS,
    SYSCON_LED_WLAN,
    SYSCON_LED_POWER,
    SYSCON_LED_BT
} SysconLed;

int syscon_ctrl_led(SysconLed led, unsigned int on);
int syscon_ctrl_hr_power(unsigned int on);
int syscon_handshake_unlock(void);
int syscon_get_pommel_version(unsigned int *version);
int syscon_get_power_status(unsigned int *status);
int syscon_get_wakeup_factor(unsigned int *factor);
int syscon_ctrl_voltage(unsigned int a0, unsigned int a1);
int syscon_reset_device(unsigned int a0, unsigned int a1);
int syscon_read_scratchpad(unsigned int src, unsigned int *dest);
int syscon_write_scratchpad(unsigned int dest, unsigned int *src);

#define SYSCON_CTRL_UP          (0x00000001)
#define SYSCON_CTRL_RIGHT       (0x00000002)
#define SYSCON_CTRL_DOWN        (0x00000004)
#define SYSCON_CTRL_LEFT        (0x00000008)
#define SYSCON_CTRL_TRIANGLE    (0x00000010)
#define SYSCON_CTRL_CIRCLE      (0x00000020)
#define SYSCON_CTRL_CROSS       (0x00000040)
#define SYSCON_CTRL_SQUARE      (0x00000080)
#define SYSCON_CTRL_SELECT      (0x00000100)
#define SYSCON_CTRL_LTRIGGER    (0x00000200)
#define SYSCON_CTRL_RTRIGGER    (0x00000400)
#define SYSCON_CTRL_START       (0x00000800)
#define SYSCON_CTRL_HOME        (0x00001000)
#define SYSCON_CTRL_HOLD        (0x00002000)
#define SYSCON_CTRL_WLAN        (0x00004000)
#define SYSCON_CTRL_HR_EJECT    (0x00008000)
#define SYSCON_CTRL_VOL_UP      (0x00010000)
#define SYSCON_CTRL_VOL_DOWN    (0x00020000)
#define SYSCON_CTRL_LCD         (0x00040000)
#define SYSCON_CTRL_NOTE        (0x00080000)
#define SYSCON_CTRL_UMD_EJECT   (0x00100000)

int syscon_get_digital_key(unsigned int *keys);

#ifdef __cplusplus
}
#endif //__cplusplus
