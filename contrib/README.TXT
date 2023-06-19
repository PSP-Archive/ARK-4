Installation On PSP

- Move or copy `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
- Move or copy `ARK_Loader` folder into `/PSP/GAME/` folder.
- Launch `ARK Loader`. It will install ARK modules on PSP Flash and boot the CFW.


  Non permanent CFW installation:

    - Delete `FLASH0.ARK` from `/PSP/SAVEDATA/ARK_01234` to prevent flashing files every time you reload the CFW.
    - Run `ARK Loader` when you boot up the system after you've fully turned off or hard reboot the device to re-enable `ARK`.


  Permanent CFW via new cIPL

    - Works on 1K models, 2K models and early 3K models (1g, 2g and 3g).
    - cIPL will not install if not compatible, so no risk if running it even if you're unsure what specific model you're using - if not compatible, use Infinity 2 (included in this download) instead.
    - Move or copy `ARK_newIPL` folder to `/PSP/GAME/` and run the program.
    - Press the corresponding button in the installation page to install the cIPL patch.
    - Warning: unlike classic cIPL, the new method can't be easily uninstalled.


  Permanent CFW via classic cIPL

    - Only works on 1K models and early 2K models (those compatible with Pandora).
    - cIPL will not install if not compatible, so no risk if running it even if you're unsure what specific model you're using - if not compatible, use Infinity 2 (included in this download) instead.
    - Move or copy `ARK cIPL` folder to `/PSP/GAME/` and run the program.
    - Press the corresponding button in the installation page to install or remove the cIPL patch.


  Permanent CFW via Infinity 2

    - Works on all PSP models on 6.60 or 6.61 firmwares.
    - Copy or move the `EBOOT.PBP` (or `EBOOT_GO.PBP` renamed to `EBOOT.PBP` if using a PSP Go) from the `Infinity` folder found in the ARK download into `/PSP/GAME/UPDATE`.
    - Run the`Infinity` app using Official Firmware to run the initial installation.
    - Run `Infinity` a second time to configure autoboot by selecting ARK within the Infinity app. Run the ARK CFW again to activate. ARK will now be automatically activated on boot up.


  Full Flash Installation

    - This allows you to install and use all of ARK's features from flash0, allowing you to entirely remove the `ARK_01234` savedata folder or memory stick.
    - Copy `ARK_Full_Installer` to `/PSP/GAME/` and run it from `ARK`.
    - It will install the necessary modules into the console's flash.
    - For recovery, you can press and hold start+select when booting up ARK or a game to temporarily disable settings and plugins.
    - Custom Firmware settings and other Custom Firmware files are installed in `ms0:/SEPLUGINS/`, which is used as the default ARK path when no savedata folder is available.


  Time Machine and Despertar del Cementerio

    - `Time Machine` allows the ability to boot the `6.61` firmware and `ARK` entirely from the Memory Stick.
    - `Despertar del Cementerio` allows the ability to revive a bricked PSP when used in combination with a `Pandora` or `Baryon Sweeper`.
    - To install `DC-ARK` you must first format the memory stick leaving enough space for the boot sector. You can use `PSP Tool` to do this.
    - You also need to be running a `CFW` to install `DC-ARK` (ARK itself or any other).
    - Copy the `ARK_DC` folder to the `/PSP/GAME/` folder and run the installer from the `XMB`.
    - Follow the instructions to install DC-ARK and create a magic memory stick.
    - On compatible models, use `Advanced VSH Menu` to create a `Pandora` battery.
    - Use a `Pandora` or `Baryon Sweeper` in combination with your newly created `Magic Memory Stick` to boot up `Despertar del Cementerio`.
    - From here you can either boot ARK from memory stick, install 6.61 Firmware with ARK on the Nand or install 6.61 Official Firmware.
    - Note that installing 6.61 firmware with ARK requires a cIPL-compatible PSP (1K and early 2K).


Installation On PS Vita


  Standalone via FastARK

    - Works on Firmware 3.60 up to 3.74, requires Henkaku/h-encore or any native hack.
    - Download and install latest `ArkFast` from this link: https://github.com/Yoti/ArkFast-4/releases/latest/.
    - Press `start` button to install the `Sasuke Commander` minigame with `ARK`.
    - If out of date with latest release, use the updater built into the `launcher`.
    - To use right analog stick (i.e. gta_remastered plugin): https://github.com/rereprep/ArkRightAnalog
    - To `exit` from a game or homebrew back to the `custom launcher`, press `L+R+Down+Start`. Works on `PSP` too.
    - Some features are not available in standalone installations, mainly the official Sony `XMB` and `PS1` games.
    - If you get a kernel exploit error, simply close the app and open it again.


  Standalone via ChovySign

    - Works on Official Firmware 2.10 up to 3.74, doesn't require Henkaku/h-encore or any native hack.
    - NOTE: you might need henkaku/h-encore to facilitate the installation process, but it won't be needed anymore after installing.
    - Requires `ChovySign`: https://silica.codes/SilicaAndPina/chovy-sign
    - Download a legit `PSP` game from `PSN`, the free demo of `Ape Quest` or `LocoRoco Midnight Carnival` is recommended.
    - Follow the instructions from the official `ChovySign` project to sign and install `ark_loader.iso` in the `PSVita/ChovySign` folder.
    - This will generate an installable `ARK Loader` in QCMA's folder as `UCJS10041` (i.e. `/Documents/PS Vita/PGAME/xxxxxxxxxxxxxxxx/UCJS10041`).
    - Follow these instructions to install `PBOOT.PBP` from `PSVita/Standalone` to the loader generated by ChovySign: https://github.com/TheOfficialFloW/Trinity#installation
    - Install the `ARK_01234` savedata folder into `/Documents/PS Vita/PSAVEDATA/xxxxxxxxxxxxxxxx/`.
    - Copy `K.BIN` from `PSVita/Standalone` folder into `ARK_01234` savedata folder.
    - Transfer the generated `ARK Loader` and `ARK_01234` savedata folders using `QCMA`.
    - To use right analog stick (i.e. gta_remastered plugin): https://github.com/rereprep/ArkRightAnalog
    - To `exit` from a game or homebrew back to the `custom launcher`, press `L+R+Down+Start`. Works on `PSP` too.
    - Some features are not available in standalone installations, mainly the official Sony `XMB` and `PS1` games.
    - If you get a kernel exploit error, simply close the app and open it again.
    - If you are running an old firmware (<3.60), you will need a spcialized `K.BIN`, either build one yourself or open an `Feature Request`.


  Adrenaline

    - Works on Firmware 3.60 up to 3.74, requires Henkaku/h-encore and `Adrenaline` (https://github.com/TheOfficialFloW/Adrenaline).
    - Install `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
    - Install `ARK_Loader` folder into `/PSP/GAME/` folder.
    - Copy `K.BIN` from `PSVita/Adrenaline` folder into `ARK_Loader` game folder.
    - Use `Adrenaline Bubbles Manager` to create an autoboot bubble for `ARK Loader`: https://github.com/ONElua/AdrenalineBubbleManager/releases
    - Note: this does not permanently modify `Adrenaline` in any way.


Updating ARK

  There are three ways to update ARK:

    Option 1: Use the `System Update` feature in the XMB. Requires your PSP/Vita to be connected to the internet.
    Option 2: Copy the `UPDATE` folder to `/PSP/GAME/` and run it.
    Option 3: Copy `ARK_01234` folder and (on PSP) run `ARK Loader` again to install new flash0 files.
