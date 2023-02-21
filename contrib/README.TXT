Installation On PSP

- Install `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
- Install `ARK_Live` folder into `/PSP/GAME/` folder.
- Launch `ARK Loader`. It will install ARK modules on PSP Flash and boot the CFW.
- To `update ARK` simply copy over the new `ARK_01234` savedata folder and run `ARK Loader` again.
- Optionally install `Advanced VSH Menu` for extra features by copying `VSHMENU.PRX` into `ARK_01234` savedata folder.


  Live CFW

  - Delete `FLASH0.ARK` from `/PSP/SAVEDATA/ARK_01234` to prevent flashing files every time you reload the CFW.
  - Run `ARK Loader` every time you power off or hard reboot the device to re-enable `ARK`.


  Permanent CFW via cIPL

  - Only works on 1K models and early 2K models (those compatible with Pandora).
  - Use `pspident` homebrew to identify your model and compatibility with custom IPL.
  - Copy `ARK cIPL` folder to `/PSP/GAME/` and run the program.
  - Press the corresponding button in the installation page to install or remove the cIPL patch.


  Permanent CFW via Infinity

  - Works on all PSP models on 6.60 or 6.61 firmwares.
  - Place the `EBOOT.PBP` (or `EBOOT_GO.PBP` renamed to `EBOOT.PBP` if using a PSP Go) from the `Infinity` folder found in the ARK download into `/PSP/GAME/UPDATE`.
  - Install `Infinity` using Official Firmware.
  - Run `Infinity` a second time to configure autoboot.



Installation On PS Vita

    - Works on 3.60 up to 3.74, either with or without Henkaku/h-encore.
    - Download a legit `PSP` game from `PSN`, the free demo of `Ape Quest` or `LocoRoco Midnight Carnival` is recommended. `Minis` are `NOT` recommended.
    - For Henkaku/h-encore users, install an ARK bubble using FastARK: https://github.com/theheroGAC/ArkFast/
    - Official Firmware users (no henkaku/h-encore) can install the PBOOT bubble manually: https://github.com/TheOfficialFloW/Trinity#preparation
    - Replace the ARK savedata folder (`ux0:pspemu/PSP/SAVEDATA/ARK_01234`) with the folder from latest release.
    - Copy `K.BIN` from VitaBubble folder into `ARK_01234` savedata folder.
    - To use right analog stick (i.e. gta_remastered plugin): https://github.com/rereprep/ArkRightAnalog
    - To `exit` from a game or homebrew back to the `custom launcher`, press `L+R+Down+Start`. Works on `PSP` too.
    - Some features are not available to Vita users, mainly the official Sony `XMB` and `PS1` games.





Advanced VSH Menu

The default VSH menu packaged in `ARK_01234` is a simplified, easier to use version of classic VSH menus.
To have a more classic VSH menu with more advanced features you can install `Advanced VSH Menu` by copying `VSHMENU.PRX` from the `AdvancedVSH` folder into `ARK_01234`.