# ARK-4

## Version 4.19.3 (2022-01-09)
- Improved Recovery menu plugin handling to properly restore comments.
- Fixed bug in plugin system where blank lines would be treated as EOF.
- Added option to hide PIC0 and PIC1 in XMB.
- Added option to disable text glow in custom launcher and recovery.
- Unlock High Memory gets disabled if UMD cache is enabled to prevent a crash.


## Version 4.19.2 (2022-12-25)
- Fixed SystemControl API and SDK libraries.
- Added "Merry Christmas!" message in custom launcher and recovery, shown every 25th of December.
- Compiled using PS4 Linux.

## Version 4.19.1 (2022-12-20)
- Refactor and cleanup of rebootex module (#60).
- Fixed case sensitivity issue in plugin system.

## Version 4.19 (2022-12-18)
- Implemented cIPL support (#58).
- Added fix for Tekken 6 when using overclock.
- Several bugfixes and improvements to rebootex.

## Version 4.18.17 (2022-09-16)
- Some small fixes, cleanup and improvements.
- Added Peace Walker theme for custom launcher.

## Version 4.18.16 (2022-08-31)
- Added patch for Vita's emulation issue with volatile RAM (Star Wars Force Unleashed, Tony Hawk Project 8, etc).
- Added patch for Vita's emulation issue with sound processing (MotorStorm Arctic Edge).

## Version 4.18.15 (2022-08-30)
- Fixed rare crash with Inferno CSO reader.
- Fixed memory leak with Inferno CSO reader when returning from sleep mode.
- Added "Show FPS" option to custom launcher.

## Version 4.18.14 (2022-08-27)
- Added support for PS Vita firmwares 3.65 up to 3.74 included.
- Some cleanup and fixes.

Note: if you wish to use an older version of ARK-4 in 3.65+ you can simply use the K.BIN provided in this release's VitaBubble folder with the savedata from the older version.

## Version 4.18.13 Rev 2 (2022-08-26)
- Added new files EXIT.PNG and PLUGINS.PNG to the theme.
- Renamed theme file from DATA.PKG to THEME.ARK for more readability.
- Renamed custom launcher from MENU.PBP to VBOOT.PBP for better compatibility with ARK-2 launchers.
- Renamed PS1 custom launcher from XMENU.PBP to XBOOT.PBP for standarization.
- New themes are now packed into the release. Credit to TheSubPlayer for MaterialDark theme.
- Fixed bug where autoboot launcher would not work with skip sony logos (#47).
- You can now run Infinity (and any other updater) from custom launcher or recovery menu, either game or browser apps (#48).

## Version 4.18.12 Rev 3 (2022-08-25)
- Fixed issue where plugins wouldn't load on certain games (#38), DLC and game sharing. For game-specific plugins too.
- Cleanup and fix cpu clock code.
- Fixed plugins manager window size when very few plugins are installed.

## Version 4.18.11 (2022-08-24)
- Added patch to hide CFW folders in retail games.
- Extra memory is now automatically unlocked for homebrews that are compatible.
- Fixed bug in Inferno Driver.

## Version 4.18.10 Rev 2 (2022-08-23)
- Implemented Sorting by Name in Custom Launcher (#42).
- Implemented automatic game loading (#44).
- Fixes and improvements to Inferno driver.
- Added option to hide Recovery Menu entry in custom launcher.
- Other fixes and improvements to custom launcher (i.e scrollbar).

## Version 4.18.9 Rev 2 (2022-08-22)
- Fixed overclock and powersave options not actually working (PSP).
- Fixed games crashing when inferno cache was disabled.
- Some other fixes and improvements in custom launcher.

## Version 4.18.8 Rev 3 (2022-08-20)
- Added "Skip Sony Logos" feature in recovery menu.
- Improved Inferno CSO reads.
- Reduced Inferno memory consumption by 14KB without affecting performance.
- Visual improvements in the custom launcher and recovery menu.

## Version 4.18.7 (2022-08-17)
- Improved custom launcher transitions between screens.
- Several other improvements and bug fixes.

## Version 4.18.6 (2022-08-14)
- You can now change the region of your UMD drive on demand via the recovery menu without having to permanently modify the flash. Allows playback of region locked UMD movies. Wait a few seconds after the XMB has loaded for the patch to be applied, then insert the UMD disc.
- NOTE: region change patch might only work on 1K and 2K models, though some 3K models are known to work too.

## Version 4.18.5 (2022-08-13)
- Added support for DTP-T1000 development kits.
- Fixed "Old Plugin Support on PSP Go".
- Fixed issue in ARK Loader.
- Added "UMD Region Free" option to recovery menu, experimental.
- Fixed the way REBOOT.BIN file is used. It is now not needed to have, but can be used to replace ARK's rebootex easily.
- Several other fixes and improvements.

Credit goes to meetpatty for adding DTP-T1000 support as well as other bugfixes and improvements.

## Version 4.18.4 (2022-08-09)
- General cleanup of core CFW files and performance improvements.
- Exit key combo (L+R+start+down) now goes to the custom launcher.

## Version 4.18.3 (2022-08-08)
- You can now scan categorized items on the custom launcher, useful for user of GCL plugin (#19).
- You can now mount and playback UMD Video ISOs via the file browser in custom launcher or recovery menu. You can't use this feature if autoboot launcher is enabled since playback is handled by the XMB.

## Version 4.18.2 (2022-08-08)
- Reduced Inferno memory consumption.
- File Browser now moves files instantly (only on PSP).

## Version 4.18.1 (2022-08-07)
- Improved file browser design (#37).
- Experimental improvements to Inferno driver.
- Fixed MacroFire plugin.
- Fixed DayViewer plugin.

## Version 4.18 (2022-07-25)
- Cleanup of LZO library, reduces memory consumption of CFW core.
- Moved VLF patches to Vita compat layer, should fix VLF issues on PSP (if there were any).
- Custom Launcher can now be set to start on file browser instead of game menu.
- Added dedicatory message every 3rd of July for Gregory Pitka.

In Loving Memory of Gregory Pitka (qwikrazor87).
You will forever be missed and your legacy will live on in this project.
Rest In Peace Dear Friend.

## Version 4.17.1 (2022-06-22)
- Fixed issues introduced in 4.17

## Version 4.17 (2022-06-08)
- Added support for 6.60 Testing Tool firmware.
- Memory usage has been reduced.
- Several other improvements.

NOTE: this release is experimental and known to have issues.

## Version 4.16 Rev 2 (2022-04-16)
- Fixed custom launcher for some of the new file formats (CSOv2, JSO, etc).

## Update 16
- Greatly improved Inferno read speeds.
- Fixed bug that affected CSO (v1 and v2) and ZSO formats.
- Added Old Plugin Support for PSP Go.
- Added "launcher" runlevel to enable/disable plugins on the custom launcher.
- Some cleanup and fixes.

## Update 15
- Fixed compatibility with Leda plugin.

## Update 14.6
- Fixed memory issues in VSH.

## Update 14.5
- Several bug fixes and improvements.
- Some more cleanup.

## Update 14.4
- Fixed DAX format games.
- Fixed Prometheus-patched games in custom launcher.

## Update 14.3
Improved Inferno Cache on all models:
- 32KB cache for PSP 1K.
- 8MB cache for 2K and newer.
- 4MB cache for Vita.

## Update 14.2
- Improved stability between the highmem, inferno cache and psp go pause features.

## Update 14.1
- Added experimental highmem on PSP 1K.

## Update 14
- Fixed highmem on PSP.
- Added experimental highmem on PS Vita.

## Update 13.2
- Added reboot runtime module support on PS Vita.
- Added ARK Version to vsh menu.

## Update 13.1
- Fixed exitgame
- Added ARK version to custom launcher and recovery.

## Update 13
- Huge cleanup of all Core CFW modules, reducing memory consumption.
- You can now disable plugins on a per-game or per-runlevel basis.

## Update 12
- More cleanup of Core CFW files.
- Fixed Popsloader V4i by PopsDeco.

## Update 11
- Huge cleanup and improvements in Core CFW modules SystemControl, Inferno and VshControl.

## Update 10
- Heavily improved SystemControl and Inferno Driver.

## Update 9.9
- Fixed reboot runtime module on PSP.
- Added fix for Team PRO's Popsloader V3 on firmware 6.61.
- Popsloader is now working with ARK!

## Update 9.8
- Added DAX and JISO support.
- Improved core compatibility by adding missing M33 functions.
- Several improvements and bugfixes.
- Now using Yoti's updated and improved savedata folder.

## Update 9.7
- Fixed standby in PS1 games.
- Fixed plugins not loading in UMD games.
- Allow ms speedup in pops.
- Added FTP client to custom launcher's file browser.
- You can now browse and copy entire files and folders from another PSP.
- And you can use the FTP client and the FTP server simultaneously.

## Update 9.6
- Fixed error when deleting ISO games in XMB.
- Prevent high memory patches in anything but homebrew.
- Improved visuals in custom launcher and recovery.
- Added "snow" and "game of life" background animations to launcher and recovery.


## Update 9.5
- Fixed memory leak when refreshing games in custom launcher.
- Fixed custom launcher crash when freeing memory from CSO games.
- Updated Inferno driver to the latest one from PRO. Should fix all ISO/CSO issues.
- Added missing SystemControl exports. Should improve homebrew compatibility.

## Update 9.4
- Fixed launcher Autoboot.
- Fixed FTP server.
- Fixed UMD games in launcher.

## Update 9.3
- Fixed mesgled patches in Stargate.
- Some small improvements in the custom launcher.
- Added file browser to recovery menu.
- Added FTP server to custom launcher.
- Fixed rename operation in file browser (launcher and recovery).
- Fixed some UMD games crashing in custom launcher when no PMF animation present.
- Some other fixes to recovery and launcher.

## Update 9.2
- Added UMD game support for custom launcher.
- Fixed memlmd patches.
- PRO Updater has been deprecated.

## Update 9.1
- Fixed issues with PSP compatibility layer that would prevent high memory from functioning correctly.

## Update 9.0:
- Added compatibility with Infinity.
- Fixed issues with PRO compatibility.

## Update 8.9:
- Created ARK_PRO_Updater, allowing to update an existing PRO installation to ARK.
- Tested and working with PRO Infinity.

## Update 8.8:
- Rebootex is now an external part of the CFW and can be easily replaced.
- Added compatibility with PRO and Adrenaline Rebootex.
- Some fixes and refactor.

## Update 8.7:
- Fixed and improved some internal APIs.
- Improved plugins menu display.
- Improved custom launcher.
---- It is now usable before all games load.
---- Added support for ZSO files in the launcher.

## Update 8.6:
- Small fix to recovery menu causing files to be written twice.
- Force recovery mode when launching recovery menu (disables plugins and settings).
- Fixed screen issues in libya2d.
- Added scrolling to plugins menu for users with many plugins installed.

## Update 8.5:
- Fixed issue when launching recovery menu from custom launcher on PSP Go causing plugins to be duplicated.
- Fixed heap memory consumption of recovery menu and custom launcher.
- Moved popsloader code back to Vita compat layer.

## Update 8.4:
- Some code cleanup regarding rebootex and loaders.
- Added built-in popsloader to run other versions of POPS installed on ARK's savedata folder.
---- PSPVMC.PRX will replace flash0:/vsh/module/libpspvmc.prx
---- POPSMAN.PRX will replace flash0:/kd/popsman.prx
---- POPS.PRX will replace flash0:/kd/pops_0Xg.prx

## Update 8.3:
- Fixed PSN eboots launched from custom menu.
- Removed Galaxy ISO Driver from the core CFW (was not used).
- Configuring rebootex for other ISO Drivers (M33, NP9660) will default to inferno instead of crashing.
- Fixed issue when disabling launcher mode that required a soft reboot to take effect.

## Update 8.2:
- Prevent some settings from running on incorrect runlevels.
- Improved file browser navigation.

## Update 8.1:
- Removed all PSP 6.60 files.
- Fixed mediasync patches on PS Vita.
- Fixed bug when opening an empty directory.
- Improved PSP IO emulation on Vita.
- Improved flash0 instalation on PSP (should be less buggy now).

## Update 8:
- Several fixes to arkMenu custom launcher.
---- Fixed file browser when doing operations on /ISO or /PSP/GAME.
---- Fixed file browser copying dialog.
---- Fixed file browser deleting dialog.
---- Fixed file browser when copying a file or folder that already exists.
---- Fixed issue launching a game when one of the menu categories was empty.
---- Added compatibility with VHBL homebrew.
---- Added Recovery Menu to list of installed homebrews.
---- Fixed issue when playing PMF that caused a memory leak.
- Added compatibility with legacy loaders for users with previous installations of ARK-2.
---- Users with previous instalations of ARK-2 for firmwares other than 3.60 must keep their previous0 K.BIN file.
- Fixed compatibility with PS Vita.
---- Per-game plugins now work well on PS Vita.
---- Fixed exitgame.
---- Fixed PSP IO emulation.
---- Ms Speedup is now a setting (as opposed to always being active).
- You can now have your PLUGINS.TXT file in /SEPLUGINS/ folder (and/or ARK's save folder).

## Update 7:
- Fixed pops plugins loaded from PSP Go internal memory (ef0).
- Fixed homebrew plugins loaded from PSP Go internal memory (ef0).
- Added "highmem" option to settings file.
- Added "mscache" option for Memory Stick speedup.
- Fixed handling of recovery mode (plugins and settings are disabled).
- Restructured installation so that core CFW files are separated from loaders.
---- The default ARK installation path is /PSP/GAME/SAVEDATA/ARK_01234/
---- This makes it compatible with Live CFW, Permanent CFW and PS Vita QCMA.
- Fixed xMenu.
- Fixed arkMenu.
- Created recovery menu for handling ARK settings and installed plugins.

## Update 6:
- Added "launcher" setting to replace XMB with a custom launcher.
--- In settings file add the following line: always, launcher, on
- Included neur0n's vMenu as a sample launcher.
- Added "disablepause" feature to settings (only works on PSP Go).
- You can now enable plugins on a per-game basis.
- Added "umd" runlevel for plugins that only load in official games.
- Added "homebrew" runlevel for plugins that only load in homebrews.

## Update 5:
- Fixed ISOs and homebrews on PSP Go.
- Added USB Charging (not for PSP 1K).
- Simplified and fixed VSH Menu for ARK.
- Added ability to create external settings file.
---- Added "overclock" option to set max CPU speed.
---- Added "powersave" option to set lower CPU speed.
---- Added "usbcharge" option.

## Update 4:
- Added ability to have a VSH menu installed externally. Classic PRO VSH menu is included.
*** please note that there's no configuration options for ARK, things like changing the CPU speed or ISO driver doesn't do anything (ARK will always enable fastest CPU speed and Inferno ISO driver).
- Added preliminary PSP Go support (untested for now).
- Fixed issue when exiting from retail UMDs.
- Exception handler now shows the module name of the crash.
- Exception handler now allows to soft reset, hard reset or shutdown the device.

## Update 3:
- Fixed issues with ISOs such as Tekken 6 and Peace Walker. All ISOs should work well now!
- Fixed issue with some homebrews like PSPFiler.
- Fixed issues with the XMB on later models. ARK now works on 2K and 3K! (PSP Go support is still not finished, PSP Street has not been tested yet).
- Added support for high memory layout on slim models for homebrews that use it.
- ARK is now stable!

## Update 2:
- Added back plugin support, including for VSH.
- Added recovery option, which lets you replace the XMB with a custom launcher (a minimalistic one is included with ARK).
- Fixed some dynamic patches.

## Update 1:
- Fixed issue when launching UMDs with ARK.
- Improved dynamic patching code for SystemControl, Inferno and Popcorn.
- Added Galaxy controller for NP9660 ISO driver (for retail PSN EBOOTs).
- Added compatibility layer for PSP-specific patches (still in development).
- Restructured PS Vita compatibility layer (still in development).

# ARK-3
- arkMenu and xMenu are now part of the project
- added PEOPS PSX SPU Plugin for partial working sound on PSX games running through PSP exploits
- added support for PSX exploits

# ARK-2
- replaced static patches with dynamic algorithms
- updated to run on higher firmwares
- replaced static loader with linkless loader
- separated ARK loader from Kernel exploit for easier deployment of ARK

# ARK-1 (PROVITA)
- initial port of PRO CFW to the PS Vita by Team PRO.
- support for ISO and CSO with the Inferno ISO Driver
- support for homebrew games and apps
- playback of soundless PSX games
