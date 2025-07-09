#include <vitasdk.h>
#include <psp2/power.h>
#include <psp2/io/devctl.h>
#include <vita2d.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <psp2/ctrl.h>
#include <psp2/appmgr.h>

#include "install.h"
#include "ui.h"

void drawBatteryAndStorage() {
    int batteryPercent = scePowerGetBatteryLifePercent();
    int isCharging = scePowerIsBatteryCharging();

    SceIoDevInfo info;
    sceIoDevctl("ux0:", 0x3001, NULL, 0, &info, sizeof(info));
    float freeSpaceGB = ((float)info.free_size) / (1024 * 1024 * 1024);

    // Determine color for free space
    uint32_t colorFree = RGBA8(0, 255, 0, 255);
    if (freeSpaceGB < 2.0f)
        colorFree = RGBA8(255, 0, 0, 255);
    else if (freeSpaceGB < 5.0f)
        colorFree = RGBA8(255, 165, 0, 255);

    // Determine color for battery
    uint32_t colorBattery = RGBA8(0, 255, 0, 255);
    if (batteryPercent <= 20)
        colorBattery = RGBA8(255, 0, 0, 255);
    else if (batteryPercent <= 30)
        colorBattery = RGBA8(255, 165, 0, 255);

    // Prepare text
    char freeText[64];
    snprintf(freeText, sizeof(freeText), "Free space: %.1f GB", freeSpaceGB);

    char batteryText[64];
    snprintf(batteryText, sizeof(batteryText), "Battery: %d%% %s",
             batteryPercent, isCharging ? "(Charging)" : "");

    // Measure width to align right
    int batteryWidth = vita2d_pgf_text_width(uiGetFont(), 1.0f, batteryText);
    int freeWidth = vita2d_pgf_text_width(uiGetFont(), 1.0f, freeText);

    // Draw both
    vita2d_pgf_draw_text(uiGetFont(), 960 - freeWidth - batteryWidth - 40, 20, colorFree, 1.0f, freeText);
    vita2d_pgf_draw_text(uiGetFont(), 960 - batteryWidth - 20, 20, colorBattery, 1.0f, batteryText);
}

int main(int argc, const char *argv[]) {
    uiInit();
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

    SceCtrlData pad;
    int selection = 0;
    const char *options[] = {
        "Install ARK-4, ARK-X and All Plugins",
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

        vita2d_pgf_draw_textf(uiGetFont(), 20, 20, RGBA8(255, 255, 255, 255), 1.2f, "FasterArk");
        vita2d_pgf_draw_textf(uiGetFont(), 20, 50, RGBA8(255, 255, 255, 255), 1.0f,
            "Select an option with Up/Down, press X to confirm:");

        for (int i = 0; i < num_options; i++) {
            uint32_t color = (i == selection) ? RGBA8(255, 0, 0, 255) : RGBA8(255, 255, 255, 255);
            vita2d_pgf_draw_textf(uiGetFont(), 60, 90 + i * 30, color, 1.0f, "%s", options[i]);
        }

        vita2d_pgf_draw_textf(uiGetFont(), 30, 90 + selection * 30, RGBA8(255, 0, 0, 255), 1.0f, "→");

        drawBatteryAndStorage();

        vita2d_end_drawing();
        vita2d_swap_buffers();

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
            displayMsg("Installing ARK-4 and ARK-X", "Installing full package...");
            doInstall();

            {
                const char *launch_options[] = { "Launch ARK-4", "Launch ARK-X" };
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

                    drawBatteryAndStorage();

                    vita2d_end_drawing();
                    vita2d_swap_buffers();

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
                    sceKernelDelayThread(1000 * 1000);
                    sceKernelExitProcess(0);
                } else {
                    sceAppMgrLaunchAppByUri(0, "psgm:play?titleid=SCPS10084"); // ARK-X
                    sceKernelDelayThread(1000 * 1000);
                    sceKernelExitProcess(0);
                }
            }
            return 0;

        case 1:
            displayMsg("Installing ARK-4", "Installing ARK-4 only (no plugins)...");
            installARK4Only();
            break;

        case 2:
            displayMsg("Installing ARK-4", "Installing ARK-4...");
            installARK4Only();
            displayMsg("Installing PS1 Plugin", "Installing PS1 plugin...");
            installPS1Plugin();
            taiReloadConfig();
            break;

        case 3:
            displayMsg("Installing Analog Plugin", "Installing Analog plugin...");
            installAnalogPlugin();
            taiReloadConfig();
            break;

        case 4:
            displayMsg("Installing PS1 Plugin", "Installing PS1 plugin...");
            installPS1Plugin();
            taiReloadConfig();
            break;

        case 5:
            displayMsg("Exit", "Exiting application...");
            return 0;
    }

    vita2d_start_drawing();
    vita2d_clear_screen();
    vita2d_pgf_draw_textf(uiGetFont(), 20, 100, RGBA8(255, 255, 255, 255), 1.0f, "Installation complete!");
    vita2d_pgf_draw_textf(uiGetFont(), 20, 140, RGBA8(255, 255, 255, 255), 1.0f, "Press X to exit.");
    drawBatteryAndStorage();
    vita2d_end_drawing();
    vita2d_swap_buffers();

    waitCross();
    return 0;
}
