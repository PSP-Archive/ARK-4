Installation On PSP

- Move or copy `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
- Move or copy `ARK_Live` folder into `/PSP/GAME/` folder.
- Launch `ARK Loader`. It will install ARK modules on PSP Flash and boot the CFW.
- To update ARK, simply copy over the new `ARK_01234` savedata folder and run `ARK Loader` again.
- Optional: Install `Advanced VSH Menu` for extra features by moving or copying `VSHMENU.PRX` into `ARK_01234` savedata folder.


  Non permanent CFW installation:

    - Delete `FLASH0.ARK` from `/PSP/SAVEDATA/ARK_01234` to prevent flashing files every time you reload the CFW.
    - Run `ARK Loader` every time you power off or hard reboot the device to re-enable `ARK`.


  Permanent CFW via cIPL

    - Only works on 1K models and early 2K models (those compatible with Pandora).
    - Use `pspident` homebrew to identify your model and compatibility with custom IPL.
    - Move or copy `ARK cIPL` folder to `/PSP/GAME/` and run the program.
    - Press the corresponding button in the installation page to install or remove the cIPL patch.


  Permanent CFW via Infinity

    - Works on all PSP models on 6.60 or 6.61 firmwares.
    - Copy or move the `EBOOT.PBP` (or `EBOOT_GO.PBP` renamed to `EBOOT.PBP` if using a PSP Go) from the `Infinity` folder found in the ARK download into `/PSP/GAME/UPDATE`.
    - Run the`Infinity` app using Official Firmware to run the initial installation.
    - Run `Infinity` a second time to configure autoboot by selecting ARK within the Infinity app. Run the ARK CFW again to activate. ARK will now be automatically activated on boot up.





Installation On PS Vita


  Standalone

    - Works on Official Firmware 3.60 up to 3.74, doesn't require Henkaku/h-encore or any native hack.
    - Download a legit `PSP` game from `PSN`, the free demo of `Ape Quest` or `LocoRoco Midnight Carnival` is recommended. `Minis` are `NOT` recommended.
    - For Henkaku/h-encore users, install an ARK bubble using [ArkFast](https://disk.yandex.ru/d/XMJpxf7RD79VLw) (manually updated, create an issue if it outdated).
    - Official Firmware users (no henkaku/h-encore) can install the PBOOT bubble manually: https://github.com/TheOfficialFloW/Trinity#preparation
    - Replace the ARK savedata folder (`ux0:pspemu/PSP/SAVEDATA/ARK_01234`) with the folder from latest release.
    - Copy `K.BIN` from `Vita/Standalone` folder into `ARK_01234` savedata folder.
    - To use right analog stick (i.e. gta_remastered plugin): https://github.com/rereprep/ArkRightAnalog
    - To `exit` from a game or homebrew back to the `custom launcher`, press `L+R+Down+Start`. Works on `PSP` too.
    - Some features are not available to Vita users, mainly the official Sony `XMB` and `PS1` games.
    - The kernel exploit is known to have stability issues, if it fails or crashes, simply restart your Vita and try again.


  Adrenaline

    - Works on Firmware 3.60 up to 3.74, requires Henkaku/h-encore and `Adrenaline` (https://github.com/TheOfficialFloW/Adrenaline).
    - Install `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
    - Install `ARK_Live` folder into `/PSP/GAME/` folder.
    - Copy `K.BIN` from `Vita/Adrenaline` folder into `ARK_Live` game folder.
    - Launch `ARK Loader` from within `Adrenaline`.
    - Create a `Savestate` using `Adrenaline`'s built-in menu and load it.
    - Use `savestates` to switch between `Adrenaline` and `ARK`.
    - Note: this does not permanently modify `Adrenaline` in any way.



Advanced VSH Menu

The default VSH menu packaged in `ARK_01234` is a simplified, easier to use version of classic VSH menus.
To have a more classic VSH menu with more advanced features you can install `Advanced VSH Menu` by moving or copying `VSHMENU.PRX` from the `AdvancedVSH` folder into `ARK_01234`.