#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#pragma once

enum Led {
    LED_MEMORY_STICK,
    LED_WLAN,
    LED_POWER,
    LED_BLUETOOTH
};

enum LedMode {
    LED_MODE_ON,
    LED_MODE_OFF,
    LED_MODE_BLINK
};

typedef struct
{
    unsigned int on_time;
    unsigned int off_time;
    unsigned int blink_time;
} LedConfig;

void led_init(void);
void led_set_mode(enum Led led, enum LedMode mode, const LedConfig *config);

#ifdef __cplusplus
}
#endif //__cplusplus
