#include <vitasdk.h>
#include <psp2/ctrl.h>
#include "ui.h"
#include "install.h"

int main(int argc, const char *argv[]) {
    uiInit();
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    int selection = 0;
    const char *options[] = {
        "Install ARK-4 and All Plugins",
        "Install Only Analog Plugin",
        "Install Only PS1 Plugin",
        "Exit"
    };
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        vita2d_start_drawing();
        vita2d_clear_screen();

        vita2d_pgf_draw_textf(uiGetFont(), 20, 40, RGBA8(255, 255, 255, 255), 1.0f,
            "Select an option with Up/Down, press X to confirm:");

        for (int i = 0; i < num_options; i++) {
            uint32_t color = (i == selection) ? RGBA8(255, 255, 0, 255) : RGBA8(255, 255, 255, 255);
            vita2d_pgf_draw_textf(uiGetFont(), 60, 80 + i * 30, color, 1.0f, "%s", options[i]);
        }
        vita2d_pgf_draw_textf(uiGetFont(), 30, 80 + selection * 30, RGBA8(255, 255, 0, 255), 1.0f, "→");

        vita2d_end_drawing();
        vita2d_swap_buffers();

        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);

        if (pad.buttons & SCE_CTRL_DOWN) {
            selection = (selection + 1) % num_options;
            sceKernelDelayThread(200 * 1000);
        } else if (pad.buttons & SCE_CTRL_UP) {
            selection = (selection - 1 + num_options) % num_options;
            sceKernelDelayThread(200 * 1000);
        } else if (pad.buttons & SCE_CTRL_CROSS) {
            break;
        }
    }

    switch (selection) {
        case 0:
            displayMsg("Installing ARK-4", "Installing ARK-4 game...");
            doInstall();

            displayMsg("Installing Plugins", "Installing all plugins...");
            installAnalogPlugin();
            installPS1Plugin();
            taiReloadConfig();
            break;
        case 1:
            displayMsg("Installing Analog Plugin", "Installing Analog plugin...");
            installAnalogPlugin();
            taiReloadConfig();
            break;
        case 2:
            displayMsg("Installing PS1 Plugin", "Installing PS1 plugin...");
            installPS1Plugin();
            taiReloadConfig();
            break;
        case 3:
            displayMsg("Exit", "Exiting application...");
            return 0;
    }

    displayMsg("Install Complete!", "Press X to exit...");
    waitCross();

    return 0;
}
