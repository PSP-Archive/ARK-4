Installation On PSP
  
- Copy `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
- Copy `ARK_Loader` folder into `/PSP/GAME/` folder.
- Launch `ARK Loader`. It will install ARK modules on PSP Flash and boot the CFW.
- At this point `ARK` will work as a `Live CFW`, meaning that `ARK Loader` will need to be run every time the console is turned off or rebooted.
- To convert `ARK` into a `Permanent CFW` you can use `cIPL`.


  Permanent CFW via cIPL

    - Works on all PSP models, including testkits and devkits.
    - Installer will choose the correct version according to your model.
    - Some models can install either the Classic cIPL or the New cIPL.
    - Copy `ARK_cIPL` folder to `/PSP/GAME/` and run the program.
    - Press the corresponding button in the installation page to install or remove the cIPL patch.

  Full Flash Installation

    - This only make sense when combined with a permanent intallation (cIPL). It doesn't make sense on PSP Go with a healthy Internal Memory.
    - This allows you to install and use all of ARK's features on the console's internal flash memory, except for Custom Launcher and translations.
    - Copy `ARK_Full_Installer` to `/PSP/GAME/` and run it from `ARK`. It will install some extra files into the console's flash.
    - If the `Custom Launcher` is not available (i.e. you delete the ARK savedata or remove the memory card), `PRO Shell` will take its place.
    - When no savedata folder is available the default ARK path used to store settings will be `ms0:/SEPLUGINS/`.
    - Even if files are stored in the console's flash memory, the ones in the savedata folder will still take priority in loading.
    - If at this point you get rid of ARK's savedata then you will have a Classic Installation (ARK will behave much like your Classic CFW such as PRO or ME).


  Time Machine and Despertar del Cementerio

    - `Time Machine` allows the ability to boot the `6.61` firmware and `ARK` entirely from the Memory Stick.
    - `Despertar del Cementerio` allows the ability to revive a bricked PSP when used in combination with a `Pandora` or `Baryon Sweeper`.
    - To install `DC-ARK` you must first format the memory stick leaving enough space for the boot sector. You can use `PSP Tool` to do this.
    - You can install `DC-ARK` either using a `PSP` or a `PC` (Windows, Mac and Linux).
    - On `PSP` you need to be running a `CFW` to install `DC-ARK` (ARK itself or any other).
    - Copy the `ARK_DC` folder to the `/PSP/GAME/` folder and run the installer from the `XMB`.
    - Follow the instructions to install DC-ARK and create a magic memory stick.
    - On `PC`, you need `Python 3` and run `MagicMemoryCreator` with `admin`/`root` priviledges.
    - On compatible models, you can use `Advanced VSH Menu` to create a `Pandora Battery`.
    - Use a `Pandora` or `Baryon Sweeper` in combination with your newly created `Magic Memory Stick` to boot up `Despertar del Cementerio`.
    - From here you can either boot ARK from memory stick, install 6.61 Firmware with ARK on the Nand or install 6.61 Official Firmware



Installation On PS Vita

There are two types of installation on PS Vita. You can use either one or both.
Stanalone works on top of the original unmodified pspemu.
You an also run ARK within Adrenaline to achieve a Hybrid CFW.

  Standalone (Official PSPEmu)

    - Works on Firmware 3.60 up to 3.74, requires Henkaku/h-encore or any native hack.
    - Download and install <a href="https://github.com/Kethen/NoPspEmuDrm/releases/tag/2025-07-02">NoPspEmuDrm</a>.
    - Copy `FasterARK.vpk` from `PSVita/` folder anywhere on your vita and install using `VitaShell`. 
    - Launch `FasterARK` and wait for the install process to finish.
    - Both `ARK` and `ARK-X` bubbles will appear in `Live Area`.
    - Use the `ARK` bubble for `PSP` games and homebrew. It can play `PS1` but with limitations related to sound playback.
    - Use the `ARK-X` bubble for `PS1` games without any sound limitations.
    - Once `ARK` bubbles are installed, you can delete `FasterARK`.
    - To `exit` from a game or homebrew back to the `custom launcher`, press `L+R+Down+Start`
        (on `ARK-X` use `L2+R2+Down+Start` or `L1+R1+Start+Down` for `VitaTV`).
    - NOTE: Some features are not available in standalone installations, mainly the official Sony `XMB`.
    - NOTE: If you want to have an `ARK` bubble that works on `Official Firmware` you must use `ChovySign` with a valid license.

  Adrenaline (Patched PSPEmu)

    - Works on Firmware 3.60 up to 3.74, requires Henkaku/h-encore and `Adrenaline` (https://github.com/isage/Adrenaline).
    - (Skip this step if `Standalone` is already installed) Install `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
    - Install `ARK_Loader` folder into `/PSP/GAME/` folder.
    - To make ARK-4 automatically load with Adrenaline there are two options.
    - (Option 1, new method) Rename `ARK_Loader` (installed previously) to `BOOT`.
        Go to Adrenaline's Recovery Menu and in the Configuration page enable the `Autorun program` option.
        This only works on newer Adrenaline versions as the feature isn't implemented in older versions.
    - (Option 2, old method) Use `Adrenaline Bubbles Manager` (https://github.com/ONElua/AdrenalineBubbleManager)
        to create an autoboot bubble for `ARK Loader`.
        This method is deprecated as it will install an older version of Adrenaline.
    - Note: this does not permanently modify `Adrenaline` in any way.


Updating ARK

  There are three ways to update ARK:

    - Option 1: Use the `System Update` feature in the XMB. Requires your PSP/Vita to be connected to the internet.
    - Option 2: Copy the `UPDATE` folder to `/PSP/GAME/` and run it.
    - Option 3: Copy `ARK_01234` folder and (on PSP) run `ARK Loader` again to install new flash0 files.
