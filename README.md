# ARK-4 e/CFW for the PSP and PS Vita.

New, updated, improved and modern `CFW` for `PSP` and `Vita`'s `ePSP`.
Simple to use and full of unique features, `ARK CFW` aims at keeping the `PSP` experience fresh.
`ARK` is now the most feature-complete `CFW` for the `PSP`, having all features from `PRO` and `ME`
as well as unique and exclusive new features not found anywhere else.

## FEATURES:

- `Core` system heavily updated from traditional CFW with new exclusive features.

- `Inferno 2` Driver compatible with all formats (`ISO`, `CSO`, `ZSO`, `JSO`, `CSOv2` and `DAX`).

- `Popcorn` controller for custom `PS1` games. Compatible with `PopsLoader` V3 and V4i.

- Built-in `No-DRM` engine `Stargate`.

- `Plugin` support for `PSP` games, `PS1` games and `VSH` (XMB), including the ability to enable and disable plugins `per-game`.

- Compatible with all `PSP` models on firmwares `6.60` and `6.61`.

- Compatible with all `PS Vita` models on firmware `2.10` up to `3.60`, either official firmware or Henkaku.

- Compatible with `6.60` `Testing Tool` Firmware and `Testkit` Units.

- Compatible with `6.60` `Development Tool` Firmware and `Devkit` Units.

- `Minimalistic`: only 6 files installed on PSP flash, CFW extensions are installed on memory stick.

- `Custom game launcher` with built-in game categories, file browser, FTP server and client, modernized look and more.

- Compatible with `PRO Online` and `Xlink Kai`.

- Compatible with Legacy Homebrew via `eLoader` and `Leda`.


## INSTALLATION

- Download the precompiled binaries from: https://github.com/PSP-Archive/ARK-4/releases/


<details>
  <summary> <b> Compiling ARK ( For Developers ) </b> </summary>
<p>

- Release: `./build.sh` 
- Debug: `./build.sh --debug`
- Manually: install the oldest possible SDK (ideally the one used to compile M33), then run `make`

</p>
</details>

<details>
  <summary> <b> On PSP </b> </summary>
<p>

- Install `ARK_01234` folder into `/PSP/SAVEDATA/` folder.
- Install `ARK_Live` folder into `/PSP/GAME/` folder.
- Launch ARK Live Loader. It will install ARK modules on PSP Flash and boot the CFW.
- To use `Infinity` place the `EBOOT.PBP` from the `Infinity` folder found in the ARK download into \
`/PSP/GAME/UPDATE`. Install `Infinity` as you normally would. \
  This will allow you to use ARK permanently and triple boot with PRO and ME.
 - If you will be using the Live loader version of ARK instead of a permanent solution, it is recommended that you delete `FLASH0.ARK` from the savedata file after the first installation and bootup of ARK, otherwise the flash0 files will be installed every time you boot back to ARK and this is detrimental to the flash even if ARK installs very little and small files. Doing this will also make the Live loader boot up ARK faster.
- To `update ARK` simply copy over the new `ARK_01234` savedata folder and run `ARK Loader` from `OFW`.

</p>
</details>

<details>
  <summary> <b> On PS Vita </b> </summary>
<p>

* For 3.60 Henkaku users:
- Download a legit `PSP` game from `PSN`, the free demo of `Ape Quest` or `LocoRoco Midnight Carnival` is recommended. `Minis` are `NOT` recommended.
- Install an ARK-4 bubble for 3.60 using FastARK: https://github.com/theheroGAC/ArkFast/releases/
- Replace the ARK savedata folder (`ux0:pspemu/PSP/SAVEDATA/ARK_01234`) with the folder from latest release.
- Copy `H.BIN` from `Vitabubble` folder of release into ARK savedata folder.
- To use right analog stick (i.e. gta_remastered plugin): https://github.com/rereprep/ArkRightAnalog
- To `exit` from a game or homebrew back to the `custom launcher`, press `L+R+Down+Start`. Works on `PSP` too.
- Some features are not available to Vita users, mainly the official Sony `XMB` and `PS1` games.

</p>
</details>

<details>
  <summary> <b> Legacy Game Exploits (PSP & Vita) </b> </summary>
<p>

- Considering the savedata exploit loads H.BIN from the savedata path.
- Copy the following files from ARK_01234 savedata folder into game exploit folder:
- H.BIN, ARK.BIN, ARK4.BIN FLASH0.ARK, VBOOT.PBP, RECOVERY.PBP, THEME.ARK and other extras.
- Don't copy K.BIN if running on an ancient firmware.

</p>
</details>

## CUSTOMIZATION

<details>
  <summary> <b> Installing Plug-Ins </b> </summary>
<p>
You can install plugins by creating a file called PLUGINS.TXT in the /SEPLUGINS/ folder and/or ARK's savedata folder.

To install plugins use the comma-separated format (CSV).
Where the header is: runlevel, path, switch.
A few samples:
- game, ms0:/seplugins/cwcheat/cwcheat.prx, enabled
- pops, ms0:/seplugins/cdda_enabler.prx, 1
- vsh, ms0:/seplugins/cxmb.prx, true
- ULUS10041, ms0:/seplugins/lcscheatdevice.prx, on

You can use the following keywords to enable a plugin:
- 1
- on
- true
- enabled
- Anything else disables the plugin

You can use the following keywords to tell ARK when the plugin loads:
- `all/always`: if either of these keywords are used, the plugin will always load.
- `umd`: plugin should only load on retail games (UMD/ISO/PSN).
- `homebrew`: plugin should only load on homebrews.
- `game`: plugin can load on both retail games and homebrews.
- `pops`: plugin only loads in PSX games.
- `vsh`: plugin only loads in the XMB.
- `launcher`: use this if the plugin should only load in the custom launcher.
- `game ID`: if you specify a game ID (i.e. `SLUS000000`), then the plugin will only load on that game.

You can also disable a plugin on certain games where they might be problematic.
To do so, just add a disable line for the specific game ID after the enable line.
For example, enable cwcheat on all retail games except for GTA LCS and VCS.
- umd, ms0:/seplugins/cwcheat/cwcheat.prx, on
- ULUS10041, ms0:/seplugins/cwcheat/cwcheat.prx, off
- ULES00502, ms0:/seplugins/cwcheat/cwcheat.prx, off

Some noteworthy plugins that are compatible with ARK include (but not limited to):
- CXMB.
- Leda.
- PopsLoader V3 and V4i.
- Custom Firmware Extender.
- PSPLink.
- GTA LCS and VCS cheadevice (including the remastered version).
- CWCheat.
- pspstates kai.

</p>
</details>

<details>
  <summary> <b> Configuration and Settings </b> </summary>
<p>

You can create a SETTINGS.TXT file using the same format as PLUGINS.TXT to enable/disable some CFW functionality on different parts of the system.
Configuration settings you can use in ARK include:

- `overclock`: use this for better performance at the expense of battery time. Sets CPU/BUS speed to 333/166.
- `powersave`: use this for better battery life at the expense of performance. Sets CPU/BUS speed to 133/66.
- `usbcharge`: enables USB charging wherever you want.
- `launcher`: replaces the XMB with a custom menu launcher.
- `disablepause`: disables the pause game feature on PSP Go.
- `highmem`: enables high memory on models above 1K.
 You should only use this on homebrew runlevel as retail games were not meant to use the extra memory,
 and this can cause issues with cheat devices or other plugins that expect games to have their data at specific memory addresses.
 - `infernocache`: enables cache for Inferno driver, improving performance of some games.
 - `oldplugin`: enables old plugins support on PSP Go (redirects `ms0` to `ef0`).
 - `skiplogos`: skips the coldboot and gameboot logos.

However, you should use the `recovery menu` to handle settings easier.

You can use the same runlevels as used in plugins to tell ARK when the settings take effect (all/always, umd, homebrew, game, pops, vsh).

For example, you can overclock to highest CPU speed like this:
- always, overclock, on

Another example, overclock only on games, use powersaving on VSH:
- game, overclock, on
- vsh, powersave, on

</p>
</details>

<details>
  <summary> <b> Custom Launcher </b> </summary>
<p>
ARK comes prepacked with a very powerful launcher with a built-in file browser and highly customizable. You can however change it to whatever you please. You can change the theme used by both the custom launcher and recovery menu by replacing THEME.ARK with your own.


Aside from the default launcher provided, there have been many  other custom launchers created by scene members, some better looking, some with more features, some more simpler, but all of them with personality, dedication and love from the community. Here is a list of all popular menus for ARK:


- ONEMenu by gdljjrod: https://github.com/ONElua/ONEmenu/releases

- vMenu by neur0n: http://www.mediafire.com/file/7acb5mhawx4gr9t/vMenu_ARK.7z/file

- yMenu by wth/yosh: https://docs.google.com/uc?export=download&id=0B0kWUCdtGmJwLUhRUlNJSWhMWVE

- gMenu by gbot: http://www.mediafire.com/file/oou5490qc99vr7d/gmenuARK.rar/file

- 138Menu by gbot: https://wololo.net/talk/viewtopic.php?f=53&t=33511

- pyMenu by Acid_Snake: https://wololo.net/talk/viewtopic.php?t=21942


Note: some of these menus have not been updated or supported by their developers in a while, they may not work well with modern ARK or real PSP hardware.
</p>
</details>


## Other

<details>
  <summary> <b> Credits </b> </summary>
<p>

- `qwikrazor87` for being such a genius and all his hard work with kernel exploits and ARK-2.

- `Team PRO` (the original developers of ARK): `Coldbird`, `hrimfaxi` and `Neur0n`.

- `TheFl0w` for his advancements and research in CFW development and overall contributions to the scene.

- `Codestation` for his incredible work improving CSO speeds and creating the ZSO format.

- `UnkownBrackets` (maxcso) for his help understanding the DAX format and Inferno speed hacks as well as creating the CSOv2 format.

- `Zer01ne`, `noname120`, `astart` and other devs that have blessed me with their knowledge and wisdom.

- `Zecoxao` for his great work creating dumpers that would allow us to archive rare firmwares.

- `balika` for his research in porting M33 to modern firmware that has helped improve compatiblity in ARK.

- Every other giant shoulder I am standing on.
</p>
</details>



<details>
  <summary> <b> Warnings </b> </summary>
<p>

- ARK comes with no warranty whatsoever. It was designed to be noob-proof, however it is possible for the universe to create an even greater noob capable of using ARK to destroy the Earth (or his PSP). I cannot be held responsible for this.

- ARK may cause ejectile malfunction if your hard drive is not hard enough.

- If this software malfunctions, you can turn it off and on again.
</p>
</details>
