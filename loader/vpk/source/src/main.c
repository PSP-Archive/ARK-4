#include <vitasdk.h>
#include <psp2/power.h>
#include <psp2/io/devctl.h>
#include <vita2d.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <psp2/ctrl.h>
#include <psp2/appmgr.h>

#include "install.h"
#include "ui.h"

void drawMenuLayout() {
    for (int y = 0; y < 64; y++) {
        uint8_t alpha = (uint8_t)(255 * (1.0f - (float)y / 64.0f * 0.7f));
        vita2d_draw_line(0, y, 960, y, RGBA8(0x20, 0x40, 0x80, alpha));
    }

    for (int y = 544 - 64; y < 544; y++) {
        uint8_t alpha = (uint8_t)(255 * (1.0f - (float)(544 - y) / 64.0f * 0.7f));
        vita2d_draw_line(0, y, 960, y, RGBA8(0x20, 0x40, 0x80, alpha));
    }

    vita2d_pgf_draw_text(uiGetFont(), 21, 31, RGBA8(0, 0, 0, 128), 1.2f, "FasterARK");
    vita2d_pgf_draw_text(uiGetFont(), 20, 30, RGBA8(255, 255, 255, 255), 1.2f, "FasterARK");

    vita2d_draw_line(0, 64, 960, 64, RGBA8(0x40, 0x80, 0xFF, 255));
    vita2d_draw_line(0, 544 - 64, 960, 544 - 64, RGBA8(0x40, 0x80, 0xFF, 255));

    vita2d_draw_line(0, 0, 20, 0, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(0, 0, 0, 20, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(940, 0, 960, 0, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(960, 0, 960, 20, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(0, 524, 20, 524, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(0, 524, 0, 544, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(940, 524, 960, 524, RGBA8(0x60, 0xA0, 0xFF, 255));
    vita2d_draw_line(960, 524, 960, 544, RGBA8(0x60, 0xA0, 0xFF, 255));
}

void drawTextCenterShadow(int y, const char* text, uint8_t r, uint8_t g, uint8_t b) {
    int width = 0, height = 0;
    vita2d_pgf_text_dimensions(uiGetFont(), 1.0f, text, &width, &height);
    int x = (960 / 2) - (width / 2);
    vita2d_pgf_draw_text(uiGetFont(), x + 1, y + 1, RGBA8(0, 0, 0, 128), 1.0f, text);
    vita2d_pgf_draw_text(uiGetFont(), x, y, RGBA8(r, g, b, 255), 1.0f, text);
}

void drawBatteryAndStorage() {
    int batteryPercent = scePowerGetBatteryLifePercent();
    int isCharging = scePowerIsBatteryCharging();

    SceIoDevInfo info;
    sceIoDevctl("ux0:", 0x3001, NULL, 0, &info, sizeof(info));
    float freeSpaceGB = ((float)info.free_size) / (1024 * 1024 * 1024);

    uint32_t colorFree = RGBA8(0, 255, 0, 255);
    if (freeSpaceGB < 2.0f)
        colorFree = RGBA8(255, 0, 0, 255);
    else if (freeSpaceGB < 5.0f)
        colorFree = RGBA8(255, 165, 0, 255);

    uint32_t colorBattery = RGBA8(0, 255, 0, 255);
    if (batteryPercent <= 20)
        colorBattery = RGBA8(255, 0, 0, 255);
    else if (batteryPercent <= 30)
        colorBattery = RGBA8(255, 165, 0, 255);

    char freeText[64];
    snprintf(freeText, sizeof(freeText), "Free space: %.1f GB", freeSpaceGB);

    char batteryText[64];
    snprintf(batteryText, sizeof(batteryText), "Battery: %d%% %s",
             batteryPercent, isCharging ? "(Charging)" : "");

    int batteryWidth = vita2d_pgf_text_width(uiGetFont(), 1.0f, batteryText);
    int freeWidth = vita2d_pgf_text_width(uiGetFont(), 1.0f, freeText);

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

        drawMenuLayout();

        vita2d_draw_rectangle(60, 100, 840, 300, RGBA8(0x20, 0x20, 0x40, 200));
        vita2d_draw_line(60, 100, 900, 100, RGBA8(0x40, 0x80, 0xFF, 255));
        vita2d_draw_line(60, 400, 900, 400, RGBA8(0x40, 0x80, 0xFF, 255));
        vita2d_draw_line(60, 100, 60, 400, RGBA8(0x40, 0x80, 0xFF, 255));
        vita2d_draw_line(900, 100, 900, 400, RGBA8(0x40, 0x80, 0xFF, 255));

        drawTextCenterShadow(120, "Select Installation Option", 0x40, 0x80, 0xFF);
        drawTextCenterShadow(150, "Use Up/Down to navigate, X to confirm", 0x80, 0xFF, 0x80);

        for (int i = 0; i < num_options; i++) {
            uint8_t r = 255, g = 255, b = 255;
            if (i == selection) {
                r = 0x60; g = 0xA0; b = 0xFF;
            }
            int width = 0, height = 0;
            vita2d_pgf_text_dimensions(uiGetFont(), 1.0f, options[i], &width, &height);
            int x = (960 / 2) - (width / 2);
            vita2d_pgf_draw_text(uiGetFont(), x + 1, 185 + i * 35 + 1, RGBA8(0, 0, 0, 128), 1.0f, options[i]);
            vita2d_pgf_draw_text(uiGetFont(), x, 185 + i * 35, RGBA8(r, g, b, 255), 1.0f, options[i]);

            if (i == selection) {
                vita2d_draw_line(75, 182 + i * 35, 95, 182 + i * 35, RGBA8(0x60, 0xA0, 0xFF, 255));
                vita2d_draw_line(95, 182 + i * 35, 90, 177 + i * 35, RGBA8(0x60, 0xA0, 0xFF, 255));
                vita2d_draw_line(95, 182 + i * 35, 90, 187 + i * 35, RGBA8(0x60, 0xA0, 0xFF, 255));
            }
        }

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

                    drawMenuLayout();

                    vita2d_draw_rectangle(80, 120, 800, 200, RGBA8(0x10, 0x30, 0x10, 220));
                    vita2d_draw_line(80, 120, 880, 120, RGBA8(0x00, 0x80, 0x00, 255));
                    vita2d_draw_line(80, 320, 880, 320, RGBA8(0x00, 0x80, 0x00, 255));
                    vita2d_draw_line(80, 120, 80, 320, RGBA8(0x00, 0x80, 0x00, 255));
                    vita2d_draw_line(880, 120, 880, 320, RGBA8(0x00, 0x80, 0x00, 255));

                    drawTextCenterShadow(140, "Installation Complete!", 0x00, 0xFF, 0x00);
                    drawTextCenterShadow(170, "Choose what to launch:", 0x80, 0xFF, 0x80);

                    for (int i = 0; i < launch_num; i++) {
                        uint8_t r = 255, g = 255, b = 255;
                        if (i == launch_sel) {
                            r = 0x00; g = 0xFF; b = 0x00;
                        }
                        int width = 0, height = 0;
                        vita2d_pgf_text_dimensions(uiGetFont(), 1.0f, launch_options[i], &width, &height);
                        int x = (960 / 2) - (width / 2);
                        vita2d_pgf_draw_text(uiGetFont(), x + 1, 205 + i * 40 + 1, RGBA8(0, 0, 0, 128), 1.0f, launch_options[i]);
                        vita2d_pgf_draw_text(uiGetFont(), x, 205 + i * 40, RGBA8(r, g, b, 255), 1.0f, launch_options[i]);

                        if (i == launch_sel) {
                            vita2d_draw_line(85, 212 + i * 40, 105, 212 + i * 40, RGBA8(0x00, 0x80, 0x00, 255));
                            vita2d_draw_line(105, 212 + i * 40, 100, 207 + i * 40, RGBA8(0x00, 0x80, 0x00, 255));
                            vita2d_draw_line(105, 212 + i * 40, 100, 217 + i * 40, RGBA8(0x00, 0x80, 0x00, 255));
                        }
                    }

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

    drawMenuLayout();

    vita2d_draw_rectangle(120, 180, 720, 140, RGBA8(0x10, 0x30, 0x10, 220));
    vita2d_draw_line(120, 180, 840, 180, RGBA8(0x00, 0x80, 0x00, 255));
    vita2d_draw_line(120, 320, 840, 320, RGBA8(0x00, 0x80, 0x00, 255));
    vita2d_draw_line(120, 180, 120, 320, RGBA8(0x00, 0x80, 0x00, 255));
    vita2d_draw_line(840, 180, 840, 320, RGBA8(0x00, 0x80, 0x00, 255));

    drawTextCenterShadow(220, "Installation Complete!", 0x00, 0xFF, 0x00);
    drawTextCenterShadow(260, "Press X to continue...", 0x80, 0xFF, 0x80);

    drawBatteryAndStorage();

    vita2d_end_drawing();
    vita2d_swap_buffers();

    waitCross();
    return 0;
}
