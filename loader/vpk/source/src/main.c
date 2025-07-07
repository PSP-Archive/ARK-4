#include <vitasdk.h>
#include <psp2/power.h>
#include <vita2d.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/ctrl.h>
#include <psp2/appmgr.h>

#include "install.h"
#include "ui.h"

int main(int argc, const char *argv[]) {
    uiInit();
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    int selection = 0;
    const char *options[] = {
        "Install ARK-4 and All Plugins",
        "Install Only ARK-4 (No Plugins)",
        "Install Only ARK-4 (No Analog Plugin)",
        "Install Only Analog Plugin",
        "Install Only PS1 Plugin",
        "Exit"
    };
    int num_options = sizeof(options) / sizeof(options[0]);

    while (1) {
        vita2d_start_drawing();
        vita2d_clear_screen();

        // Title
        vita2d_pgf_draw_textf(uiGetFont(), 20, 20, RGBA8(255, 255, 255, 255), 1.2f, "FasterArk");

        // Instructions
        vita2d_pgf_draw_textf(uiGetFont(), 20, 50, RGBA8(255, 255, 255, 255), 1.0f,
            "Select an option with Up/Down, press X to confirm:");

        // Menu options
        for (int i = 0; i < num_options; i++) {
            uint32_t color = (i == selection) ? RGBA8(255, 0, 0, 255) : RGBA8(255, 255, 255, 255);
            vita2d_pgf_draw_textf(uiGetFont(), 60, 90 + i * 30, color, 1.0f, "%s", options[i]);
        }

        // Red arrow
        vita2d_pgf_draw_textf(uiGetFont(), 30, 90 + selection * 30, RGBA8(255, 0, 0, 255), 1.0f, "→");

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
        case 0: // Full install: ARK-4 + all plugins (ARK-X included)
            displayMsg("Installing ARK-4 and ARK-X", "Installing full package...");
            doInstall();

            // Show launch menu (ARK-4 or ARK-X)
            {
                const char *launch_options[] = {
                    "Launch ARK-4",
                    "Launch ARK-X"
                };
                int launch_sel = 0;
                int launch_num = sizeof(launch_options) / sizeof(launch_options[0]);

                while (1) {
                    vita2d_start_drawing();
                    vita2d_clear_screen();

                    vita2d_pgf_draw_textf(uiGetFont(), 20, 40, RGBA8(255, 255, 255, 255), 1.0f,
                        "Installation complete! Choose what to launch:");

                    for (int i = 0; i < launch_num; i++) {
                        uint32_t color = (i == launch_sel) ? RGBA8(255, 0, 0, 255) : RGBA8(255, 255, 255, 255);
                        vita2d_pgf_draw_textf(uiGetFont(), 60, 90 + i * 30, color, 1.0f, "%s", launch_options[i]);
                    }

                    vita2d_pgf_draw_textf(uiGetFont(), 30, 90 + launch_sel * 30, RGBA8(255, 0, 0, 255), 1.0f, "→");

                    vita2d_end_drawing();
                    vita2d_swap_buffers();

                    SceCtrlData pad;
                    sceCtrlPeekBufferPositive(0, &pad, 1);

                    if (pad.buttons & SCE_CTRL_DOWN) {
                        launch_sel = (launch_sel + 1) % launch_num;
                        sceKernelDelayThread(200 * 1000);
                    } else if (pad.buttons & SCE_CTRL_UP) {
                        launch_sel = (launch_sel - 1 + launch_num) % launch_num;
                        sceKernelDelayThread(200 * 1000);
                    } else if (pad.buttons & SCE_CTRL_CROSS) {
                        break;
                    }
                }

                if (launch_sel == 0) {
                    sceAppMgrLaunchAppByUri(0, "psgm:play?titleid=NPUZ01234"); // ARK-4
                } else {
                    sceAppMgrLaunchAppByUri(0, "psgm:play?titleid=SCPS10084"); // ARK-X
                }
            }
            return 0;

        case 1: // Only ARK-4 (no plugins)
            displayMsg("Installing ARK-4", "Installing ARK-4 only (no plugins)...");
            installARK4Only();
            break;

        case 2: // ARK-4 + PS1 plugin only
            displayMsg("Installing ARK-4", "Installing ARK-4...");
            installARK4Only();

            displayMsg("Installing PS1 Plugin", "Installing PS1 plugin...");
            installPS1Plugin();
            taiReloadConfig();
            break;

        case 3: // Analog plugin only
            displayMsg("Installing Analog Plugin", "Installing Analog plugin...");
            installAnalogPlugin();
            taiReloadConfig();
            break;

        case 4: // PS1 plugin only
            displayMsg("Installing PS1 Plugin", "Installing PS1 plugin...");
            installPS1Plugin();
            taiReloadConfig();
            break;

        case 5: // Exit
            displayMsg("Exit", "Exiting application...");
            return 0;
    }

    // Show completion screen for non-launch paths
    vita2d_start_drawing();
    vita2d_clear_screen();
    vita2d_pgf_draw_textf(uiGetFont(), 20, 100, RGBA8(255, 255, 255, 255), 1.0f, "Installation complete!");
    vita2d_pgf_draw_textf(uiGetFont(), 20, 140, RGBA8(255, 255, 255, 255), 1.0f, "Press X to exit.");
    vita2d_end_drawing();
    vita2d_swap_buffers();

    waitCross();
    return 0;
}
