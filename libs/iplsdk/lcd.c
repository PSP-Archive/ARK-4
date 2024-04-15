#include "lcd.h"
#include "phatlcd.h"
#include "hibari.h"
#include "samantha.h"
#include "tmdlcd.h"
#include "streetlcd.h"

#include <model.h>

const LcdDriver *lcd_driver(void)
{
    switch (model_get_identity()->model) {
        case PSP_MODEL_01G:
            return &g_phat_lcd_driver;

        case PSP_MODEL_02G:
            return &g_hibari_driver;

        case PSP_MODEL_03G:
        case PSP_MODEL_04G:
        case PSP_MODEL_05G:
        case PSP_MODEL_09G:
            return &g_samantha_driver;

        case PSP_MODEL_07G:
            return &g_tmdlcd_driver;

        case PSP_MODEL_11G:
            return &g_street_lcd_driver;
    }

    return NULL;
}
