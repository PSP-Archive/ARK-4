# ARK-4 e/CFW for the PSP and PS Vita.

New, updated, improved and modern `Custom Firmware` for the `PSP` and `PS Vita`'s `ePSP`.
Simple to use and full of unique features, `ARK CFW` aims at keeping the `PSP` experience fresh. Being a successor to `PRO` and `ME`,
`ARK` is now the most feature-complete `CFW` for the `PSP`, having all features from classic `CFW`
as well as unique and exclusive new features not found anywhere else.

<br>

<a style="font-size: 18px;" href="https://discord.gg/bePrj9W">Join the PSP Homebrew Community Discord</a>
<br>
<label style="text-decoration: underline; font-size: 14px;">We are located in the <b>#ark-cfw channel</b></label>
<br>
<br>
<br>



### Installation Guide located in WIKI
<a style="font-weight: bold; font-size:32px; text-decoration: underline;" href="https://github.com/PSP-Archive/ARK-4/wiki">ARK-4 WIKI</a>
<br>
<br>
<br>

### Table of Contents (Quick Links)
  * [FEATURES:](#features)
  * [Installation on PSP](#Installation-on-PSP)
  * [Installation on PS Vita](#installation-on-ps-vita)
  * [Changelog](#changelog)
  * [Warnings](#warnings)

## FEATURES:

- `Core` system heavily updated from traditional CFW with new exclusive features.

- `Inferno 2` Driver compatible with all formats (`ISO`, `CSO`, `ZSO`, `JSO`, `CSOv2` and `DAX`).

- `Popcorn` controller for custom `PS1` games. Compatible with `PopsLoader` V3 and V4i.

- Built-in `No-DRM` engine `Stargate`, fixes many anti-CFW games. Compatible with `npdrm_free` and `nploader`.

- `Plugin` support for `PSP` games, `PS1` games and `VSH` (XMB), including the ability to enable and disable plugins `per-game`.

- `Region Free` playback of `UMD Video` on all PSP models. Change the region of your `UMD` drive on-the-fly.

- Compatible with all `PSP` models on firmwares `6.60` and `6.61`.

- Compatible with all `PS Vita` models on firmware `2.10` up to `3.74`, either official firmware or via `Adrenaline`.

- Compatible with `6.60` `Testing Tool` Firmware and `Testkit` Units.

- Compatible with `6.60` `Development Tool` Firmware and `Devkit` Units.

- Can be fully installed and booted on memory stick in compatible models via `Time Machine`.

- Can be used to unbrick compatible PSP models using `Despertar del Cementerio` in combination with a `Pandora` or `Baryon Sweeper`.

- `Minimalistic`: only 6 files installed on PSP flash, CFW extensions are installed on memory stick.

- Resistant to `soft-bricks`, easy to recover from bad configurations with a new and improved `Recovery` app.

- `Custom game launcher` with built-in game categories, file browser, `FTP` server and client, modernized look and more.

- Fully configurable via the `XMB`.

- Compatible with `PRO Online` and `Xlink Kai`.

- Compatible with Legacy Homebrew via `eLoader` and `Leda`. Compatible with the KXploit format.

- Compatible with `cIPL` and `Infinity 2` bootloaders for permanent CFW.

- `OTA` updates. Fully updateable via the internet.


![CFW Settings](.github/screenshots/cfw_settings.png "CFW Settings")
![Advanced VSH Menu](.github/screenshots/advanced_vsh.png "CFW Settings")
![Infinity](.github/screenshots/infinity.png "CFW Settings")


#### Compiling ARK ( For Developers )
<p>

    Build script will allow you to use the correct SDK that ARK was built with.

- Docker container: `docker pull docker.io/krazynez/ark-4:latest`

<p><b>These are utilized either with cloning the repo or using the Docker container</b></p>

- Release: `./build.sh` 
- Debug: `./build.sh --debug`
- Manually: install the oldest possible SDK (ideally the one used to compile M33), then run `make`

Use `-h` or `--help` to show all available flags 

</p>

#### Installation On PSP

##### The instructions are as follows:
  
- Move or copy `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
- Move or copy `ARK_Loader` folder into `/PSP/GAME/` folder.
- Launch `ARK Loader`. It will install ARK modules on PSP Flash and boot the CFW.
- At this point `ARK` will work as a `Live CFW`, meaning that `ARK Loader` will need to be run every time the console is turned off or rebooted.
- To convert `ARK` into a `Permanent CFW` you can use either `cIPL` or `Infinity`, along with the `Full Installer` for a complete permanent experience.


##### Permanent CFW via cIPL

- `New cIPL`: works on `1g`, `2g` and `3g` models on `6.61`.
- `Classic cIPL`: works on `Pandora` compatbile models (`1g` and early `2g`) on `6.60`, `6.61` or `6.60 Testing Tool`.
- `DevTool cIPL`: works on `DTP-T1000` devkit units on `6.60 Development Tool` firmware.
- cIPL will not install if not compatible, so no risk if running it even if you're unsure what specific model you're using - if not compatible, use `Infinity 2` instead.
- Move or copy either `ARK_cIPL` folder to `/PSP/GAME/` and run the program.
- Press the corresponding button in the installation page to install or remove the cIPL patch.
- Warning: unlike classic cIPL, the new cIPL method can't be easily uninstalled. To revert to offical IPL, use ChronoSwitch to reinstall 6.61 OFW.


##### Permanent CFW via Infinity 2

- Works on all PSP models on 6.60 or 6.61 firmwares.
- Copy or move the `EBOOT.PBP` (or `EBOOT_GO.PBP` renamed to `EBOOT.PBP` if using a PSP Go) from the `Infinity` folder found in the ARK download into `/PSP/GAME/UPDATE`.
- Run the`Infinity` app using Official Firmware to run the initial installation.
- Run `Infinity` a second time to configure autoboot by selecting ARK within the Infinity app. Run the ARK CFW again to activate. ARK will now be automatically activated on boot up.


##### Full Flash Installation

- This allows you to install and use all of ARK's features on the console's internal flash memory, allowing you to entirely remove the `ARK_01234` savedata folder or memory stick.
- Copy `ARK_Full_Installer` to `/PSP/GAME/` and run it from `ARK`. It will install some extra files into the console's flash.
- If the `Custom Launcher` is not available (i.e. you delete the ARK savedata or remove the memory card), `PRO Shell` will take its place.
- When no savedata folder is available the default ARK path used to store settings will be `ms0:/SEPLUGINS/`.
- Even if files are stored in the console's flash memory, the ones in the savedata folder will still take priority in loading.


##### Time Machine and Despertar del Cementerio

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


#### Installation On PS Vita


##### Standalone (Official PSPEmu)

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
- NOTE: If you want to have an `ARK` bubble that works on `Official Firmware` you must use `ChovySign` with a valid license. Instructions in Wiki.


##### Adrenaline (Patched PSPEmu)

- Works on Firmware 3.60 up to 3.74, requires Henkaku/h-encore and `Adrenaline` (https://github.com/TheOfficialFloW/Adrenaline).
- (Skip this step if `Standalone` is already installed) Install `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
- Install `ARK_Loader` folder into `/PSP/GAME/` folder.
- Use `Adrenaline Bubbles Manager` to create an autoboot bubble for `ARK Loader`: https://github.com/ONElua/AdrenalineBubbleManager/releases
- Note: this does not permanently modify `Adrenaline` in any way.


#### Updating ARK

##### There are three ways to update ARK:

- Option 1: Use the `System Update` feature in the XMB. Requires your PSP/Vita to be connected to the internet.
- Option 2: Copy the `UPDATE` folder to `/PSP/GAME/` and run it.
- Option 3: Copy `ARK_01234` folder and (on PSP) run `ARK Loader` again to install new flash0 files.

#### Changelog

- `ARK-1`: original port/rewrite of `PRO CFW` for the `PS Vita`. Codenamed `PROVITA`. Source code can be found here: https://github.com/PSP-Archive/ARK-1-PROVita-
- `ARK-2`: dynamic patching allows it to work with most of `PS Vita` firmwares. Source code can be found here: https://github.com/PSP-Archive/ARK-2
- `ARK-3`: device-specific runtime allows `ARK` to run in multiple scenarios (`ePSP` and `ePSX`). Source code can be found here: https://github.com/PSP-Archive/ARK-3
- `ARK-4`: ported to the original `PSP`. Huge amounts of improvements and fixes over previous versions.

`Note`: this is a simplified `changelog`, for a full version you can visit here: https://github.com/PSP-Archive/ARK-4/blob/main/CHANGELOG.md


#### Warnings
<p>

- ARK comes with no warranty whatsoever. It was designed to be noob-proof, however it is possible for the universe to create an even greater noob capable of using ARK to destroy the Earth (or his PSP). I cannot be held responsible for this.

- ARK may cause ejectile malfunction if your hard drive is not hard enough.

- If this software malfunctions, you can turn it off and on again.
</p>
