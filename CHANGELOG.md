# ARK-4

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
