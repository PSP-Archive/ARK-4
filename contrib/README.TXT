Installation On PSP

The instructions are as follows:
  
- Move or copy `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
- Move or copy `ARK_Loader` folder into `/PSP/GAME/` folder.
- Launch `ARK Loader`. It will install ARK modules on PSP Flash and boot the CFW.
- At this point `ARK` will work as a `Live CFW`, meaning that `ARK Loader` will need to be run every time the console is turned off or rebooted.
- To convert `ARK` into a `Permanent CFW` you can use either `cIPL` or `Infinity`, along with the `Full Installer` for a complete permanent experience.


  Permanent CFW via cIPL

    - `New cIPL`: works on `1g`, `2g` and `3g` models on `6.61`.
    - `Classic cIPL`: works on `Pandora` compatbile models (`1g` and early `2g`) on `6.60`, `6.61` or `6.60 Testing Tool`.
    - `DevTool cIPL`: works on `DTP-T1000` devkit units on `6.60 Development Tool` firmware.
    - cIPL will not install if not compatible, so no risk if running it even if you're unsure what specific model you're using - if not compatible, use `Infinity 2` instead.
    - Move or copy either `ARK_cIPL` folder to `/PSP/GAME/` and run the program.
    - Press the corresponding button in the installation page to install or remove the cIPL patch.
    - Warning: unlike classic cIPL, the new cIPL method can't be easily uninstalled. To revert to offical IPL, use ChronoSwitch to reinstall 6.61 OFW.


  Permanent CFW via Infinity 2

    - Works on all PSP models on 6.60 or 6.61 firmwares.
    - Copy or move the `EBOOT.PBP` (or `EBOOT_GO.PBP` renamed to `EBOOT.PBP` if using a PSP Go) from the `Infinity` folder found in the ARK download into `/PSP/GAME/UPDATE`.
    - Run the`Infinity` app using Official Firmware to run the initial installation.
    - Run `Infinity` a second time to configure autoboot by selecting ARK within the Infinity app. Run the ARK CFW again to activate. ARK will now be automatically activated on boot up.


  Full Flash Installation

    - This allows you to install and use all of ARK's features on the console's internal flash memory, allowing you to entirely remove the `ARK_01234` savedata folder or memory stick.
    - Copy `ARK_Full_Installer` to `/PSP/GAME/` and run it from `ARK`. It will install some extra files into the console's flash.
    - If the `Custom Launcher` is not available (i.e. you delete the ARK savedata or remove the memory card), `PRO Shell` will take its place.
    - When no savedata folder is available the default ARK path used to store settings and other Custom Firmware files will be `ms0:/SEPLUGINS/`.
    - Even if files are stored in the console's flash memory, the ones in the savedata folder will still take priority in loading.


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


  Standalone (Official PSPEmu)

    - Works on Firmware 3.60 up to 3.74, requires Henkaku/h-encore or any native hack.
    - Download and install <a href="https://github.com/LiEnby/NoPspEmuDrm/releases">NoPspEmuDrm</a> (you can use `AutoPlugin`).
    - Copy `FasterARK.vpk` from `PSVita/` folder anywhere on your vita and install using `VitaShell`. 
    - Open `FasterARK` and wait for the install process to finish.
    - Both `ARK` and `ARK-X` bubbles will appear in `Live Area`.
    - Use the `ARK` bubble for `PSP` games and homebrew. It can play `PS1` but with limitations related to sound playback.
    - Use the `ARK-X` bubble for `PS1` games without any sound limitations.
    - Once `ARK` bubbles are installed, you can delete `FasterARK`.
    - To `exit` from a game or homebrew back to the `custom launcher`, press `L+R+Down+Start` (on `ARK-X` use `L2+R2+Down+Start` or `L1+R1+Start+Down` for `VitaTV`).
    - NOTE: Some features are not available in standalone installations, mainly the official Sony `XMB` and `Force Extra RAM` setting (needed for `PRO Online`).
    - NOTE: If you want to have an `ARK` bubble that works on `Official Firmware` you must use `ChovySign` with a valid license.

  Adrenaline (Patched PSPEmu)

    - Works on Firmware 3.60 up to 3.74, requires Henkaku/h-encore and `Adrenaline` (https://github.com/TheOfficialFloW/Adrenaline).
    - (Skip this step if `Standalone` is already installed) Install `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
    - Install `ARK_Loader` folder into `/PSP/GAME/` folder.
    - Use `Adrenaline Bubbles Manager` to create an autoboot bubble for `ARK Loader`: https://github.com/ONElua/AdrenalineBubbleManager/releases
    - Note: this does not permanently modify `Adrenaline` in any way.


Updating ARK

  There are three ways to update ARK:

    - Option 1: Use the `System Update` feature in the XMB. Requires your PSP/Vita to be connected to the internet.
    - Option 2: Copy the `UPDATE` folder to `/PSP/GAME/` and run it.
    - Option 3: Copy `ARK_01234` folder and (on PSP) run `ARK Loader` again to install new flash0 files.
