#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef enum
{
    LCD_DRIVER_RES_OK,
    LCD_DRIVER_ERR_NOT_IMPL,
    LCD_DRIVER_ERR_BAD_MODEL,
} LcdDriverResult;

typedef struct LcdDriver
{
    LcdDriverResult (* init)(void);
    LcdDriverResult (* set_mode)(int mode, unsigned int xres, unsigned int yres, unsigned int stride, unsigned int format);
    LcdDriverResult (* set_backlight_brightness)(unsigned int brightness);
    LcdDriverResult (* enable_display)(void);
    LcdDriverResult (* disable_display)(void);
    LcdDriverResult (* on_interrupt)(void);
} LcdDriver;

const LcdDriver *lcd_driver(void);

#ifdef __cplusplus
}
#endif //__cplusplus
