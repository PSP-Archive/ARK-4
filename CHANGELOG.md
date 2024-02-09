# ARK Changelog

## Version 4.20.68 (2024-02-06)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42068
- Released `ARK-X` for `PS Vita`.
#### Rev 6
- Fixed exit on PS Vita.
#### Rev 5
- Fixed screen not rendering when exiting back to launcher in `ARK-X`.
- Fixed crash in `ARK-X` when no `PS1` games are installed.
#### Rev 4
- Fixed bug in `FasterARK` that would corrupt taiHEN config.
#### Rev 3
- `cwcheat` now displays correctly in `ARK-X`.
#### Rev 2
- `ARK-X` installation is now handled by `FasterARK`, making it considerably easier to install and use.
- Added patch to prevent crashing in TwinBee Portable when the system language is not set to Japanese or English.
- Lots of code cleanup.

## Version 4.20.67 (2024-01-18)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42067
- Rev 7: fixed and improved exit key combo (L+R+Start+Down). Updated SystemControl libraries.
- Rev 6: fixed crash when returning from sleep in some games (i.e. `Lego Star Wars II`) due to unchecked zero-division error. Improved `Disable Analog Stick` for some games.
- Rev 5: improved (classic) recovery menu; better visuals and improved code.
- Rev 4: you can now boot `Custom Launcher` in `Recovery` mode by holding `R Trigger` when it is loading (essentially turning it into the original ARK Recovery Menu). Added default `SETTINGS.TXT` file with recommended settings for initial installations.
- Rev 3: fixed extra XMB icons not showing up on PSP Street models. Fixed Russian translation. Added new Turquoise theme for custom launcher.
- Rev 2: cleanup of `PRO Shell`. Fix for `PSN` games on older launchers. Proper alignment of icon in `Custom Launcher`.
- Rev 1: improved API call for detection of toolkits (can now distinguish TestKit from DevKit). Improved launcher menu navigation.
- Merged `Recovery Menu` into `Custom Launcher`, leaving `Classic Recovery` as the default recovery menu.
- Updated kernel exploit used on `PSP` and `Adrenaline`.
- Added ability to install plugins using `PRO Shell` (Classic Recovery Menu).
- Cleanup and improved `VSH Menu`.
- Added ability to hide entries in `Advanced VSH Menu` by pressing `L` trigger, use `R` trigger to unhide all hidden entries.
- Fixed `Import Classic Plugins` feature.
- Fixed issue in plugins manager of classic recovery.
- Added ability to force open `VSH Menu` by pressing `L+R+Select` anywhere in the `XMB`.
- Several more cleanup and fixes.

## Version 4.20.66 (2023-10-24)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42066
- Rev 9: added option to enable `QA Flags`, which will allow `Debug` settings to appear in the `XMB`. Added `Custom App` Launcher. If ARK detects an `EBOOT.PBP` in either `/PSP/APP/` it display a icon under Custom Launcher icon stating `Custom App`.
- Rev 8: fixed bug that would prevent installing `new cIPL` on classic models (1K and early 2K). Fixed bug in the plugins manager of classic recovery menu.
- Rev 7: fixed `MP3` audio issues in `Custom Launcher`.
- Rev 6: several cleanup, refactor and small fixes.
- Rev 5: you can now change background image of `Custom Launcher` and `Recovery` by placing a file named `BG.PNG` in `ARK_01234`.
- Rev 4: added `alternative` kernel exploit for better stability.
- Rev 3: unified all kernel exploits (`K.BIN`) into one, simplifying the installation process across all devices (https://github.com/PSP-Archive/ARK-4/pull/273).
- Rev 2: fixed bug in file browser's options menu.
- Rev 1: fixed bug that would allow cIPL to be installed on 4g models (not yet compatible), causing a brick.
- Simplified `cIPL` installer into a single app. Automatically detects device and model to install the proper version.
- Allow custom translations for `XMB Control` and `VSH Menu`.
- Intensity of text glow in `Custom Launcher` can now be changed.
- `Recovery Menu` will properly refresh list of plugins and settings.
- Fixed `per-game` settings on `PSN` eboots.
- Improved translations.
- Other small fixes.

## Version 4.20.65 (2023-10-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42065
- Rev 5: brings back `per-game` plugins (and settings) on `PS1` games, feature that was previously removed by mistake. 
- Rev 4: fixed crash with exit combo. Fixed per-game plugins on PSN eboots. Fix crash when using `No UMD` on PSN eboots.
- Rev 3: `Disable UMD Drive` will now physically disable the drive, not just software-only.
- Rev 2: `Custom Firmware Settins` and `Plugins Manager` in the `XMB` will now be located in the `Extras` section when available. 
- Rev 1: undo mistake that disabled the code that fixes 6.60 plugins on 6.61
- Fixed regression that would prevent loading `ISO` on `PSP Go` without M2 Memory Stick.
- Improved support for `Leda` plugin on `PSP Go`.
- Launching homebrew on `PSP Go` via the `XMB` is now done exactly like `PRO/ME`. This means that homebrew launched on `ef0` (Internal Memory) will have `ms0 to ef0 redirection` enabled. To launch homebrew on `ef0` without redirecting `ms0` you can use `Custom Launcher` (with the `redirect` option turned `off` in `menu settings`).
- Improved `OTA` updater for `DC-ARK` installations.
- `Custom Firmware Settings` and `Plugins Manager` in the `XMB` is now located under the `Game` section.
- Other minor visual improvements to the `XMB`.
- Other small fixes and improvements.

## Version 4.20.64 (2023-09-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42064
- Rev 3: fixed regression that would prevent `UMD Video ISO` from working. Added patch to fix the screen in `cwcheatpops` plugin.
- Rev 2: improve `Memory Stick Speedup` support on `PSP Go` models.
- Rev 1: revert back to `PRO/ARK` mscache code using `ME`'s configuration. Provides the best results between compatibility and stability.
- Added multi-runlevels support for plugins (i.e "game vsh, ms0:/seplugins/etc, on").
- Added support for changing `CPU Clock` in `PS Vita`. Note: the `Overclock` setting (333MHz) is renamed to `PSP Overclock` to avoid confusion with native Vita Overclock (444MHz-555MHz).
- `Classic Recovery Menu` can now be used on `PS Vita` when the regular recovery is not found (removed).
- Added new setting to `Disable UMD Drive` on `PSP` (except for `Go`).
- Added new setting to `Disable Analog Stick`.
- Improved stability of `Custom Launcher` by forcing `malloc/free` to be `thread-safe`.
- Implemented new `oe_malloc/oe_free` functions that consume less memory.
- Replaced `msstor_cache` code (`Memory Stick Speedup` setting) from `PRO/ARK` with the one from `ME`. Fixes issues with `cwcheat` and lowers memory consumption.
- Fixed issue that would cause `MP3 Playlists` to loop current song instead of playing the next song.

## Version 4.20.63 (2023-08-11):
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42063
- Rev 2: Fixed and improved support for PSP Go models.
- Rev 1: You can now select USB Device in Classic Recovery Menu, allows to mount flash memory for recovery operations.
- Created `Classic Recovery Menu` for `PSP` systems with `Full Installation`, which is used when there is no `Recovery App` available (i.e. no `Memory Stick` inserted or no `ARK_01234` savedata present).
- Improved `Custom Firmware Settings` in both `Recovery Menu` and `XMB`.
- Fixed and improved `Cyrillic` fonts (https://github.com/PSP-Archive/ARK-4/pull/198).
- Several fixes related to `Full Installations`.
- Cleanup of development libraries.
- Several other fixes and improvements.

## Version 4.20.62 (2023-08-07):
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42062
- Added support for `cIPL` for the `DTP-T1000` DevKit.
- You can now change between the `New` and `Classic` menu designs in `Advanced VSH Menu`.
- Several small fixes and improvements.

## Version 4.20.61 (2023-07-06):
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42061
- (Rev 3) Several bug fixes and improvements.
- (Rev 2) Cleanup and fixed `VSH Menu`. Improved `Peops SPU` configuration.
- (Rev 1) Fixed issue with `Development Tool Firmware` (devkits). 
- You can now play `PS1` games on `PS Vita Standalone` installations. Uses the original `Pops` from `PSP` and the `Peops SPU` plugin. 
- Several small fixes and improvements.

## Version 4.20.60 (2023-07-06):
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42060
- (Rev 6) Fixed issue where launcher would not load. Refactor of plugins code in core CFW module.
- (Rev 5) Fixed per-game settings and plugins on physical `UMD` discs. Fixed folder detection in `File Browser`.
- (Rev 4) Fixed issues with physical `UMD` discs. In `VSH Menu`, behavior of the X/O button is now the same as the `XMB`.
- (Rev 3) Fixed issues with non-latin files.
- (Rev 2) Heavily improved loading times of `File Browser`.
- You can now toggle `USB` connection in `File Browser` (`Launcher`/`Recovery`).
- Implemented UNIX-style hidden files/folders in `Custom Launcher` (and `Recovery`).
- `Custom Laucher` (and `Recovery`) will now remember last folder used in `File Browser`.
- Updated and added new translations.
- Improved lots of text displayed in the `Custom Launcher` (and `Recovery`) when using translations.
- `File Browser` can now display game icons and animations.
- Added option to disable file size calculation in `File Browser`.
- A progress bar is now displayed when downloading an update in the `Custom Launcher`.
- Fixed graphical glitch when loading the `Custom Launcher` or `Recovery`.
- Several other fixes and improvements.

## Version 4.20.59 (2023-06-27):
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42059
- Heavily improved and refactor of VSH Menu.
- Text Editor will now ask you to save when closing a modified file.
- You can now copy game information to use on Text Editor.
- Several other fixes and improvements.

## Version 4.20.58 (2023-06-24)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42058
- Added support for `PS Vita Standalone` installations using `NoPspEmuDrm`.
- Fixed `Inferno Cache` on `PS Vita Standalone` installations.
- Added new setting `Balanced Energy Mode`, forces the `CPU` clock to `222MHz`.
- You can now use `per-game` settings.
- You can now use `per-game` settings and plugins on `PS1` games.
- Fixed `potential` compatibility issues with `some` plugins. Confirmed with `Macrofire` plugin.

## Version 4.20.57 (2023-06-22)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42057
- Heavily improved `Waves` animation in `launcher`/`recovery`.
- `Custom Launcher`, `Recovery Menu` and `VSH Menu` can now be translated. Included are a few translations.
- Improved support for files with non-latin characters in `launcher`/`recovery`.
- Added new themes `Jurassic Park` and `Ubuntu`.
- Some other bug fixes and improvements.

## Version 4.20.56 (2023-06-10)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42056
- Fixed bug introduced in version 4.20.40 that would crash PS1 games on Adrenaline installations.

## Version 4.20.55 (2023-06-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42055
- Fixed crash when launching games from `XMB` introduced in the `4.20.53` update.
- Cleaned up of `ChovySign` dummy `ISO` for `Standalone` installations on `PS Vita`.

## Version 4.20.54 Rev 2 (2023-06-07)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42054-r2
- Added support for using `ChovySign` to create `Standalone` installations on `PS Vita`.

## Version 4.20.54 (2023-06-07)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42054
- Fixed crash in `PS Vita` with `Standalone` installations.
- Improved stability of kernel exploit (`K.BIN` file) for `PS Vita Standalone` installations.
- Fixed updater in `PS Vita Standalone` installations.
- Added new theme `yokai` made by a user.

## Version 4.20.53 (2023-06-06)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42053
- Improved `Advanced VSH Menu` options in `Adrenaline` installations.
- Improved `Advanced VSH Menu` options in `PSP Go` models.
- Fixed several issues in `(Advanced) VSH Menu` that would prevent configuration from being updated.
- Fixed and improved random color selection in `(Advanced) VSH Menu`.
- Added support for `PBOOT.PBP` updates and `Prometheus`-patched games in the `Boot Random ISO` feature of `Advanced VSH Menu`
- Improved memory management in the `XMB` when reading `ISO/CSO` files.
- Added new themes `Windows 7`, `Windows XP` and `CyanogenPSP` for `Custom Launcher`/`Recovery`.
- Severeal more cleanup and improvements.

## Version 4.20.52 (2023-06-05)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42052
- Added patch to remove annoying "overclocked" message in ATV Offroad Fury Pro.
- You can now change the size of the main menu in Custom Launcher or Recovery.
- Added Korean translation to XMB Control.
- Fixed graphical glitch in Advanced VSH Menu on the Convert Battery option.
- Some other cleanup in the compatibility layers.

## Version 4.20.51 (2023-05-28)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42051
- Fixed crash when exiting custom launcher on PS Vita.
- Added Dutch translations to XMB Control.

## Version 4.20.50 (2023-05-26)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42050
- Completely refactored all core code related to settings handling. It is now faster and consumes less resources.
- Added ability to change `VSH` and `UMD` region in `Advanced VSH Menu`, changing `UMD` region is instant while changing `VSH` region requires a reset.
- Added `PSN` patches and fixes from `PRO` CFW.
- Other cleanup and improvements.

## Version 4.20.40 (2023-05-24)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42040
- You can now install all external modules in flash0, allowing for a full permanent installation.
- Fixed issue when booting some ISOs in the `custom launcher`.
- Added ability to `restart` and `shutdown` on `custom launcher` (for `PSP` only).
- Added configuration to change behavior of Start button in `custom launcher`, options are: Disabled, Boot Current Game, Boot Last Game, Boot Random ISO.
- Holding the `L-trigger` while loading the `custom launcher` will autoboot the last played game.
- Cleanup of `pentazemin` compatibility layer and other parts of the `Custom Firmware`.

## Version 4.20.35 (2023-05-16)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42035
- Improved `660on661` patch. Fixes crashes and other issues in homebrew such as `chronoswitch` and others.
- Fixed crash in `DJ Max`. Still a heavy Anti-Piracy game.
- `Free space` available (on ms0 or ef0) is now shown in `file browser`.
- You can now boot into `OFW` using `new cIPL` (holding `Home` button).

## Version 4.20.34 (2023-05-15)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42034
- Fixed black screen when launching PS1 games and homebrew on PSP Go.
- Fixed slowdowns in the custom launcher when previewing a game.

## Version 4.20.33 (2023-05-12)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42033
- Ported over the new cIPL reset hack from DC-M33 project. Allows for permament Custom IPL installations on 3g models (early 3K).
- Fixed issues with Inferno driver.
- Added Simplified Chinese translation in the XMB.
- Other fixes and improvements.

## Version 4.20.20 (2023-05-10)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42020
- Improved memory management in the XMB, fixes issues such as crashes with too many games loaded.
- Cleanup and fix Inferno driver.
- Added option to Hide DLC in the XMB.
- Added battery percentage to launcher and recovery.
- Other fixes, cleanup and improvements.

## Version 4.20.19 (2023-05-09)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42019
- You can now update ARK using the Custom Launcher.

## Version 4.20.18 (2023-05-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42018
- Fixed bug introduced in 4.20.17 that would cause retail games to show "corrupt save data" error.

## Version 4.20.17 (2023-05-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42017
- Added patch that unlocks higher firmware features on older homebrews (more noticeably audiovideo codecs).
- Fixed and improved music player. You can now select more than one MP3 file to create a playlist.
- Other general fixes and improvements.

## Version 4.20.16 (2023-05-07)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42016
- Legacy 1.50 homebrew in `KXploit` format is now properly deleted in `XMB` and `launcher`.
- Added `rename` option to game menu in custom launcher.
- Cleaned up `pentazemin` compatibility layer.
- Allow `Stargate` no-DRM engine to run in `UMD` games. Fixes crashing in some `UMD`s.
- Other general fixes and improvements.

## Version 4.20.15 (2023-05-06)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42015
- Added date and time to launcher/recovery.
- Improved music player in file browser, now you can use triangle button to exit the music player while the music keeps playing.
- Added options to Game Menu in custom launcher by pressing L trigger.
- Some visual improvements to the custom launcher.

## Version 4.20.14 (2023-05-05)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42014
- Fixed LEDA plugin crashing on DC-ARK.
- Added hotkeys to disable plugins and/or settings. Press and hold start (disable plugins) and/or select (disable settings) when the console starts up, or a game is being boot, or exiting from a game, to temporarily disable plugins/settings.

## Version 4.20.13 (2023-05-04)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42013
- Fixed exit game freezing on some homebrew in PS Vita (standalone and adrenaline).
- Fixed regression that would mess up plugin loading on PSP Go.
- Added patch that fixes 6.60 plugins and homebrew on 6.61 firmware.
- Added Official Firmware version to launcher and recovery. It is also shown in System Information (XMB) when the option to hide MAC address is enabled.
- Added option to completely disable LEDs on PSP.

## Version 4.20.12 (2023-05-04)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42012
- Fixed XMB USB device mount in Adrenaline compatibility mode.
- Fixed CWCHEAT POPS in Adrenaline compatibility mode.
- Fixed Autoboot Launcher in Adrenaline compatibility mode.

## Version 4.20.11 (2023-05-03)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42011
- Fixed Force Extra Memory and Inferno Cache on Adrenaline.

## Version 4.20.10 (2023-05-02)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r42010
- Fixed sleep/resume crashes in Pentazemin compatibility layer.
- Fixed Boot Random ISO in PS Vita.
- Compatibility with Adrenaline is now Stable!

## Version 4.20.09 (2023-05-02)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4209
- Fixed and improved System Update patches.
- Update server URL is now stored in UPDATER.TXT inside the ARK_01234 folder. Removing this file will leave the official System Update.
- Updater is now ready, you can update to this release from 4.20.08 using System Update feature in the XMB.

## Version 4.20.08 (2023-05-01)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4208
- Updating ARK from the XMB is now working. There is no actual update available now, but it will run a test app if you try to update.

## Version 4.20.07 (2023-04-30)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4207
- Fixed regression introduced to Inferno ISO driver that would increase memory consumption.
- Replaced XMB CSO reader code with one based on Inferno's, improves speed and fixes issues.
- Improved ISO/CSO scanning in custom launcher.
- Improved waiting icon animations in launcher/recovery.
- Added preliminary code for Custom Update. Does not get applied yet.
- Updated themes: black, blue, blue2 and red.

## Version 4.20.06 (2023-04-29)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4206
- Cleaned up VSH Menu, fixed scePaf imports (fixes crash on Development Tool units).
- Fixed and improved Inferno CSO reader, specially noticeable on games like God of War Ghost of Sparta. CSO files can't go any faster though (due to slow decompression algorithm), it is recommended to use JSO or ZSO for near ISO speeds.
- Improved (fixed?) crashing issue when sleep/resume in Crisis Core.
- Added animation to waiting icon on custom launcher or recovery.
- Re-wrote "Boot Random Game" option, it's faster, no longer consumes any extra RAM and works with categorized /ISO/ folder. 
- Some other fixes and improvements.

## Version 4.20.05 (2023-04-27)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4205
- Fixed crash when launching a 1.50 homebrew with KXploit format on PSP.
- Improved image viewer scrolling.
- Improved themes preview function. You can now use themes without installing them.
- Added disc0:/ to File Browser, lets you browse the files on a UMD (not available on Vita or PSP Go).
- You can now choose where to extract .zip and .rar files (current folder or root folder).
- Some other fixes and cleanup.

## Version 4.20.04 (2023-04-26)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4204
- Fixed crashes in Custom Launcher if you have a 1.50 homebrew in kxploit format (MyHomebrew%/EBOOT.PBP).
- The XMB will now hide corrupt icons from 1.50 homebrew in kxploit format (MyHomebrew%/EBOOT.PBP).
- Improved scrolling in custom launcher and recovery.
- Added missing sounds to BadgerOS and BadgerOS Sprunk themes.
- You can now view images in PNG, JPEG or BMP format in the File Browser.
- You can now play music in MP3 format in the File Browser.
- Added new icons for music and photo files in File Browser.

## Version 4.20.03 (2023-04-25)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4203
- Fixed regression with Inferno driver.
- Added option to Hide MAC Address in XMB.
- Ported over PRO's Libertas MAC Spoofer. Lets you change the PSP mac address using /seplugins/mac.txt. Always enabled when the file is found with a valid MAC address.
- Improved scrollbar in launcher/recovery.
- Several cleanup and fixes.

## Version 4.20.02 (2023-04-24)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4202
- You can now preview and install themes directly from the launcher/recovery File Browser. The theme is loaded instantly.
- FTP Client: allow connecting to FTP servers via their name instead of the IP address.
- FTP Client: add ability to specify the port of the FTP server (21 by default if non specified) as well as username and password (anonymous connection used if none entered).

## Version 4.20.01 (2023-04-23)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4201
- Fixed crash when opening "Custom Firmware Settings" in the XMB on a PSP Go.
- Fixed issue that would make the internal memory of a PSP Go disappear after exiting a game.
- Fixed issue with USB Device change in PSP Go.
- Other fixes related to PSP Go.

## Version 4.20 (2023-04-20)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r420
- Ported Time Machine and Despertar del Cementerio to ARK, allowing to unbrick PSPs and install and run the 6.61 firmware (plus ARK) on a memory stick.
- Created the Pentazemin compatibility layer that allows ARK to run inside Adrenaline, creating a Hybrid Custom Firmware that combines the features of both Custom Firmwares.
  Note: extremely experimental feature that's still in development and there are known bugs.
- Created a new VSH Menu that combines both the Simple VSH Menu and the Advanced VSH Menu, no longer having to install it from a separate file to access its features.
- Re-wrote all static patches in the PSP compatibility layer to dynamic ones, fixing any issues with Testkits and Devkits.

## Version 4.19.16 (2023-02-24)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r41916
- File browser now has icons for compatible file formats (iso, cso, zso, jso, dax, pbp, prx, zip, rar, txt, cfg and ini).

## Version 4.19.15 (2023-02-22)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r41915
- Fixed all games reporting "not enough space" even when there is.
- Changed default launcher/recovery theme.

## Version 4.19.14 (2023-02-22)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r41914
- You can now use slim colors on PSP 1K.
- Added partial support for files with non-latin1 characters in recovery/launcher.
- Added new CFW setting "Block hibernation deletion on PSP Go", available in XMB and recovery menu.
- Fixed issues with "Skip Sony Logos", such as compatibility with Testing Tool and Development Tool firmwares.
- Renamed launcher/recovery settings file from ARKMENU.CFG to ARKMENU.BIN to prevent confusion since it's not a text file.
- Added new features to Advanced VSH Menu: USB Readonly mode, background color, activate flash and wma players, swap X/O buttons and delete hibernation.

## Version 4.19.13 (2023-02-21)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r41913
- You can now remove plugins using the Plugin Manager in the XMB or Recovery Menu. This doesn't delete the plugin or its files, only removes it from PLUGINS.TXT, to fully uninstall the plugin you can use the file manager to delete its files.
- You can now edit text files (.txt, .cfg or .ini) using the file browser.

## Version 4.19.12 (2023-02-19)
- fixed ISOs with non-latin1 characters (only on XMB though, launcher/recovery still has issues with them).
- launcher and recovery will now hide options that aren't available depending on device and model (ftp, ef0, etc) and CFW settings.
- added option to change VSH region.
- added support for DLC and Updates on ISO files.

## Version 4.19.11 (2023-02-17)
- You can now install plugins using the file browser (Recovery Menu or Custom Launcher).
- Created Advanced VSH Menu, based on PRO VSH Menu and Ultimate VSH Menu, can be optionally installed to obtain advanced features in the XMB, including: change USB device, view available ISO drivers, mount Video ISO files, convert battery from normal to pandora and viceversa.
- Added version number to Advanced VSH Menu and fixed typo.

## Version 4.19.10 (2023-02-10)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r41910
- Added new keywords for plugin system: `psx` and `ps1` as alternative to `pops`; `xmb` as alternative to `vsh` and `psp` as alternative to `umd`.
- Added option to redirect ms0 to ef0 on custom launcher.
- Added translations for XMB Control.
- Fixed incompatibility between XMB Control and XMB Item Hider plugin (v1.3-fix3).
- XMB Control will now hide useless settings depending on device.
- Fixed issues when unmounting UMD Video ISO.
- Added new themes "Black" and "Matrix".

## Version 4.19.9 (2023-02-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4199
- UMD Region Change is finally working! You can now play region locked UMD movies on any PSP model from any region. To change the UMD drive region, enter recovery menu and scroll to the last option, choose your region and exit recovery menu. Wait for the XMB to finish loading (the memory stick LED stops blinking) and insert the UMD you want to play.

## Version 4.19.8 (2023-02-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4198
- Fixed regression introduced in 4.19.7 where having Inferno Cache and High Memory enabled at the same time would cause a crash.
- Fixes and improvements to the plugin system.
- You can now configure Custom Firmware settings and plugins on the XMB (changes take effect on next game boot or reboot).

## Version 4.19.7 (2023-01-22)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4197
- Fixed patch to allow usermode to access high memory.
- Improved Matrix animation.

## Version 4.19.6 (2023-01-14)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4196
- Several fixes and improvements to the custom launcher and recovery.
- Fixed and improved "Matrix" animation.
- Added new "Hacker" and "BSoD" animations.

## Version 4.19.5 (2023-01-12)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4195
- Fixed issue where VSH plugins would not load after pausing game in PSP Go.
- Recovery menu no longer deletes custom config lines.
- You can now enable/disable DLC scanning in custom launcher (previously always enabled).

## Version 4.19.4 (2023-01-09)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4194
- Fixed device autodetection in ARK Live Loader.
- Added easter egg.

## Version 4.19.3 (2023-01-09)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4193
- Improved Recovery menu plugin handling to properly restore comments.
- Fixed bug in plugin system where blank lines would be treated as EOF.
- Added option to hide PIC0 and PIC1 in XMB.
- Added option to disable text glow in custom launcher and recovery.
- Unlock High Memory gets disabled if UMD cache is enabled to prevent a crash.


## Version 4.19.2 (2022-12-25)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4192
- Fixed SystemControl API and SDK libraries.
- Added "Merry Christmas!" message in custom launcher and recovery, shown every 25th of December.
- Compiled using PS4 Linux.

## Version 4.19.1 (2022-12-20)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r41901
- Refactor and cleanup of rebootex module (#60).
- Fixed case sensitivity issue in plugin system.

## Version 4.19 (2022-12-18)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r419
- Implemented cIPL support (#58).
- Added fix for Tekken 6 when using overclock.
- Several bugfixes and improvements to rebootex.

## Version 4.18.17 (2022-09-16)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r1817
- Some small fixes, cleanup and improvements.
- Added Peace Walker theme for custom launcher.

## Version 4.18.16 (2022-08-31)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r1816
- Added patch for Vita's emulation issue with volatile RAM (Star Wars Force Unleashed, Tony Hawk Project 8, etc).
- Added patch for Vita's emulation issue with sound processing (MotorStorm Arctic Edge).

## Version 4.18.15 (2022-08-30)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r1815
- Fixed rare crash with Inferno CSO reader.
- Fixed memory leak with Inferno CSO reader when returning from sleep mode.
- Added "Show FPS" option to custom launcher.

## Version 4.18.14 (2022-08-27)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r1814
- Added support for PS Vita firmwares 3.65 up to 3.74 included.
- Some cleanup and fixes.

Note: if you wish to use an older version of ARK-4 in 3.65+ you can simply use the K.BIN provided in this release's VitaBubble folder with the savedata from the older version.

## Version 4.18.13 (2022-08-26)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r1813
- Added new files EXIT.PNG and PLUGINS.PNG to the theme.
- Renamed theme file from DATA.PKG to THEME.ARK for more readability.
- Renamed custom launcher from MENU.PBP to VBOOT.PBP for better compatibility with ARK-2 launchers.
- Renamed PS1 custom launcher from XMENU.PBP to XBOOT.PBP for standarization.
- New themes are now packed into the release. Credit to TheSubPlayer for MaterialDark theme.
- Fixed bug where autoboot launcher would not work with skip sony logos (#47).
- You can now run Infinity (and any other updater) from custom launcher or recovery menu, either game or browser apps (#48).

## Version 4.18.12 (2022-08-25)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r1812
- Fixed issue where plugins wouldn't load on certain games (#38), DLC and game sharing. For game-specific plugins too.
- Cleanup and fix cpu clock code.
- Fixed plugins manager window size when very few plugins are installed.

## Version 4.18.11 (2022-08-24)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r1811
- Added patch to hide CFW folders in retail games.
- Extra memory is now automatically unlocked for homebrews that are compatible.
- Fixed bug in Inferno Driver.

## Version 4.18.10 (2022-08-23)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r1810
- Implemented Sorting by Name in Custom Launcher (#42).
- Implemented automatic game loading (#44).
- Fixes and improvements to Inferno driver.
- Added option to hide Recovery Menu entry in custom launcher.
- Other fixes and improvements to custom launcher (i.e scrollbar).

## Version 4.18.9 (2022-08-22)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r189
- Fixed overclock and powersave options not actually working (PSP).
- Fixed games crashing when inferno cache was disabled.
- Some other fixes and improvements in custom launcher.

## Version 4.18.8 (2022-08-20)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r188
- Added "Skip Sony Logos" feature in recovery menu.
- Improved Inferno CSO reads.
- Reduced Inferno memory consumption by 14KB without affecting performance.
- Visual improvements in the custom launcher and recovery menu.

## Version 4.18.7 (2022-08-17)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r187
- Improved custom launcher transitions between screens.
- Several other improvements and bug fixes.

## Version 4.18.6 (2022-08-14)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r186
- You can now change the region of your UMD drive on demand via the recovery menu without having to permanently modify the flash. Allows playback of region locked UMD movies. Wait a few seconds after the XMB has loaded for the patch to be applied, then insert the UMD disc.
- NOTE: region change patch might only work on 1K and 2K models, though some 3K models are known to work too.

## Version 4.18.5 (2022-08-13)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r185
- Added support for DTP-T1000 development kits.
- Fixed "Old Plugin Support on PSP Go".
- Fixed issue in ARK Loader.
- Added "UMD Region Free" option to recovery menu, experimental.
- Fixed the way REBOOT.BIN file is used. It is now not needed to have, but can be used to replace ARK's rebootex easily.
- Several other fixes and improvements.

Credit goes to meetpatty for adding DTP-T1000 support as well as other bugfixes and improvements.

## Version 4.18.4 (2022-08-09)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r184
- General cleanup of core CFW files and performance improvements.
- Exit key combo (L+R+start+down) now goes to the custom launcher.

## Version 4.18.3 (2022-08-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r183
- You can now scan categorized items on the custom launcher, useful for user of GCL plugin (#19).
- You can now mount and playback UMD Video ISOs via the file browser in custom launcher or recovery menu. You can't use this feature if autoboot launcher is enabled since playback is handled by the XMB.

## Version 4.18.2 (2022-08-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r182
- Reduced Inferno memory consumption.
- File Browser now moves files instantly (only on PSP).

## Version 4.18.1 (2022-08-07)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r181
- Improved file browser design.
- Experimental improvements to Inferno driver.
- Fixed MacroFire plugin.
- Fixed DayViewer plugin.

## Version 4.18.0 (2022-07-25)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r180
- Cleanup of LZO library, reduces memory consumption of CFW core.
- Moved VLF patches to Vita compat layer, should fix VLF issues on PSP (if there were any).
- Custom Launcher can now be set to start on file browser instead of game menu.
- Added dedicatory message every 3rd of July for Gregory Pitka.

In Loving Memory of Gregory Pitka (qwikrazor87).
You will forever be missed and your legacy will live on in this project.
Rest In Peace Dear Friend.

## Version 4.17.1 (2022-06-22)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r171
- Fixed issues introduced in 4.17

## Version 4.17.0 (2022-06-08)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r170
- Added support for 6.60 Testing Tool firmware.
- Memory usage has been reduced.
- Several other improvements.
*NOTE: this release is experimental and known to have issues.

## Version 4.16.0 (2022-04-16)
- https://github.com/PSP-Archive/ARK-4/releases/tag/r160-2
- Fixed custom launcher for some of the new file formats (CSOv2, JSO, etc).
- Greatly improved Inferno read speeds.
- Fixed bug that affected CSO (v1 and v2) and ZSO formats.
- Added Old Plugin Support for PSP Go.
- Added "launcher" runlevel to enable/disable plugins on the custom launcher.
- Some cleanup and fixes.

## Version 4.15.0
- https://github.com/PSP-Archive/ARK-4/releases/tag/r150
- Fixed compatibility with Leda plugin.

## Version 4.14.6
- https://github.com/PSP-Archive/ARK-4/releases/tag/r146
- Fixed memory issues in VSH.

## Version 4.14.5
- https://github.com/PSP-Archive/ARK-4/releases/tag/r145
- Several bug fixes and improvements.
- Some more cleanup.

## Version 4.14.4
- https://github.com/PSP-Archive/ARK-4/releases/tag/r144
- Fixed DAX format games.
- Fixed Prometheus-patched games in custom launcher.

## Version 4.14.3
- https://github.com/PSP-Archive/ARK-4/releases/tag/r143
- Improved Inferno Cache on all models:
- 32KB cache for PSP 1K.
- 8MB cache for 2K and newer.
- 4MB cache for Vita.

## Version 4.14.2
- https://github.com/PSP-Archive/ARK-4/releases/tag/r142
- Improved stability between the highmem, inferno cache and psp go pause features.

## Version 4.14.1
- https://github.com/PSP-Archive/ARK-4/releases/tag/r141
- Added experimental highmem on PSP 1K.

## Version 4.14.0
- https://github.com/PSP-Archive/ARK-4/releases/tag/r140
- Fixed highmem on PSP.
- Added experimental highmem on PS Vita.

## Version 4.13.2
- https://github.com/PSP-Archive/ARK-4/releases/tag/r132
- Added reboot runtime module support on PS Vita.
- Added ARK Version to vsh menu.

## Version 4.13.1
- https://github.com/PSP-Archive/ARK-4/releases/tag/r131
- Fixed exitgame
- Added ARK version to custom launcher and recovery.

## Version 4.13.0
- https://github.com/PSP-Archive/ARK-4/releases/tag/r130
- Huge cleanup of all Core CFW modules, reducing memory consumption.
- You can now disable plugins on a per-game or per-runlevel basis.

## Version 4.12.0
- https://github.com/PSP-Archive/ARK-4/releases/tag/r120
- More cleanup of Core CFW files.
- Fixed Popsloader V4i by PopsDeco.

## Version 4.11.0
- https://github.com/PSP-Archive/ARK-4/releases/tag/r110
- Huge cleanup and improvements in Core CFW modules SystemControl, Inferno and VshControl.

## Version 4.10.0
- https://github.com/PSP-Archive/ARK-4/releases/tag/r100
- Heavily improved SystemControl and Inferno Driver.

## Version 4.9.9
- https://github.com/PSP-Archive/ARK-4/releases/tag/r99
- Fixed reboot runtime module on PSP.
- Added fix for Team PRO's Popsloader V3 on firmware 6.61.
- Popsloader is now working with ARK!

## Version 4.9.8
- https://github.com/PSP-Archive/ARK-4/releases/tag/r98
- Added DAX and JISO support.
- Improved core compatibility by adding missing M33 functions.
- Several improvements and bugfixes.
- Now using Yoti's updated and improved savedata folder.

## Version 4.9.7
- https://github.com/PSP-Archive/ARK-4/releases/tag/r97-5
- Fixed standby in PS1 games.
- Fixed plugins not loading in UMD games.
- Allow ms speedup in pops.
- Added FTP client to custom launcher's file browser.
- You can now browse and copy entire files and folders from another PSP.
- And you can use the FTP client and the FTP server simultaneously.

## Version 4.9.6
- https://github.com/PSP-Archive/ARK-4/releases/tag/r96
- Fixed error when deleting ISO games in XMB.
- Prevent high memory patches in anything but homebrew.
- Improved visuals in custom launcher and recovery.
- Added "snow" and "game of life" background animations to launcher and recovery.


## Version 4.9.5
- https://github.com/PSP-Archive/ARK-4/releases/tag/r95
- Fixed memory leak when refreshing games in custom launcher.
- Fixed custom launcher crash when freeing memory from CSO games.
- Updated Inferno driver to the latest one from PRO. Should fix all ISO/CSO issues.
- Added missing SystemControl exports. Should improve homebrew compatibility.

## Version 4.9.4
- https://github.com/PSP-Archive/ARK-4/releases/tag/r94
- Fixed launcher Autoboot.
- Fixed FTP server.
- Fixed UMD games in launcher.

## Version 4.9.3
- https://github.com/PSP-Archive/ARK-4/releases/tag/r93-2
- Fixed mesgled patches in Stargate.
- Some small improvements in the custom launcher.
- Added file browser to recovery menu.
- Added FTP server to custom launcher.
- Fixed rename operation in file browser (launcher and recovery).
- Fixed some UMD games crashing in custom launcher when no PMF animation present.
- Some other fixes to recovery and launcher.

## Version 4.9.2
- https://github.com/PSP-Archive/ARK-4/releases/tag/r92
- Added UMD game support for custom launcher.
- Fixed memlmd patches.
- PRO Updater has been deprecated.

## Version 4.9.1
- https://github.com/PSP-Archive/ARK-4/releases/tag/r91
- Fixed issues with PSP compatibility layer that would prevent high memory from functioning correctly.

## Version 4.9.0:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r90-rev1
- Added compatibility with Infinity.
- Fixed issues with PRO compatibility.

## Version 4.8.9:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r89
- Created ARK_PRO_Updater, allowing to update an existing PRO installation to ARK.
- Tested and working with PRO Infinity.

## Version 4.8.8:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r88
- Rebootex is now an external part of the CFW and can be easily replaced.
- Added compatibility with PRO and Adrenaline Rebootex.
- Some fixes and refactor.

## Version 4.8.7:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r87
- Fixed and improved some internal APIs.
- Improved plugins menu display.
- Improved custom launcher.
---- It is now usable before all games load.
---- Added support for ZSO files in the launcher.

## Version 4.8.6:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r86-rev5
- Small fix to recovery menu causing files to be written twice.
- Force recovery mode when launching recovery menu (disables plugins and settings).
- Fixed screen issues in libya2d.
- Added scrolling to plugins menu for users with many plugins installed.
- Improved plugins menu.
- Improved custom launcher.
- Fixed strange bug that caused slowdowns in the menu.
- Added support for ZSO in the custom launcher.


## Version 4.8.5:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r85
- Fixed issue when launching recovery menu from custom launcher on PSP Go causing plugins to be duplicated.
- Fixed heap memory consumption of recovery menu and custom launcher.
- Moved popsloader code back to Vita compat layer.

## Version 4.8.4:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r84
- Some code cleanup regarding rebootex and loaders.
- Added built-in popsloader to run other versions of POPS installed on ARK's savedata folder.
---- PSPVMC.PRX will replace flash0:/vsh/module/libpspvmc.prx
---- POPSMAN.PRX will replace flash0:/kd/popsman.prx
---- POPS.PRX will replace flash0:/kd/pops_0Xg.prx

## Version 4.8.3:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r83
- Fixed PSN eboots launched from custom menu.
- Removed Galaxy ISO Driver from the core CFW (was not used).
- Configuring rebootex for other ISO Drivers (M33, NP9660) will default to inferno instead of crashing.
- Fixed issue when disabling launcher mode that required a soft reboot to take effect.

## Version 4.8.2:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r83
- Prevent some settings from running on incorrect runlevels.
- Improved file browser navigation.

## Version 4.8.1:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r81
- Removed all PSP 6.60 files.
- Fixed mediasync patches on PS Vita.
- Fixed bug when opening an empty directory.
- Improved PSP IO emulation on Vita.
- Improved flash0 instalation on PSP (should be less buggy now).

## Version 4.8.0:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r8
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

## Version 4.7.0:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r7
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

## Version 4.6.0:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r6
- Added "launcher" setting to replace XMB with a custom launcher.
--- In settings file add the following line: always, launcher, on
- Included neur0n's vMenu as a sample launcher.
- Added "disablepause" feature to settings (only works on PSP Go).
- You can now enable plugins on a per-game basis.
- Added "umd" runlevel for plugins that only load in official games.
- Added "homebrew" runlevel for plugins that only load in homebrews.

## Version 4.5.0:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r5
- Fixed ISOs and homebrews on PSP Go.
- Added USB Charging (not for PSP 1K).
- Simplified and fixed VSH Menu for ARK.
- Added ability to create external settings file.
---- Added "overclock" option to set max CPU speed.
---- Added "powersave" option to set lower CPU speed.
---- Added "usbcharge" option.

## Version 4.4.0:
- https://github.com/PSP-Archive/ARK-4/releases/tag/r4
- Added ability to have a VSH menu installed externally. Classic PRO VSH menu is included.
*** please note that there's no configuration options for ARK, things like changing the CPU speed or ISO driver doesn't do anything (ARK will always enable fastest CPU speed and Inferno ISO driver).
- Added preliminary PSP Go support (untested for now).
- Fixed issue when exiting from retail UMDs.
- Exception handler now shows the module name of the crash.
- Exception handler now allows to soft reset, hard reset or shutdown the device.

## Version 4.3.0:
- Fixed issues with ISOs such as Tekken 6 and Peace Walker. All ISOs should work well now!
- Fixed issue with some homebrews like PSPFiler.
- Fixed issues with the XMB on later models. ARK now works on 2K and 3K! (PSP Go support is still not finished, PSP Street has not been tested yet).
- Added support for high memory layout on slim models for homebrews that use it.
- ARK is now stable!

## Version 4.2.0:
- Added back plugin support, including for VSH.
- Added recovery option, which lets you replace the XMB with a custom launcher (a minimalistic one is included with ARK).
- Fixed some dynamic patches.

## Version 4.1.0:
- Fixed issue when launching UMDs with ARK.
- Improved dynamic patching code for SystemControl, Inferno and Popcorn.
- Added Galaxy controller for NP9660 ISO driver (for retail PSN EBOOTs).
- Added compatibility layer for PSP-specific patches (still in development).
- Restructured PS Vita compatibility layer (still in development).

## Version 4.0.0:
- Initial experimental release working on PSP 3K.

# ARK-4
- Added compatibility with real PSP hardware.
- Refactored Vita and VitaPOPS compatibility.
- Improved arkMenu (custom launcher).
- Re-wrote Inferno CSO reader (adds compatibility with other formats as well as better speeds and less memory consumption).
- Many bug fixes and improvements.

# ARK-3
- https://github.com/PSP-Archive/ARK-3
- arkMenu and xMenu are now part of the project
- added PEOPS PSX SPU Plugin for partial working sound on PSX games running through PSP exploits
- added support for PSX exploits (TN-X patches)

# ARK-2
- https://github.com/PSP-Archive/ARK-2
- replaced static patches with dynamic algorithms
- updated to run on higher firmwares
- replaced static loader with linkless loader
- separated ARK loader from Kernel exploit for easier deployment of ARK

# ARK-1 (PROVITA)
- https://github.com/PSP-Archive/ARK-1-PROVita-
- initial rewrite of PRO CFW to the PS Vita by Team PRO.
- support for ISO and CSO with the Inferno ISO Driver
- support for homebrew games and apps
- playback of soundless PSX games
