# General Information about ARK

## BOOT SEQUENCE

1. Code execution in User Mode.
1.2. Load H.BIN anywhere in RAM.
1.3. H.BIN loads ARK.BIN in scratchpad.
1.4. ARK.BIN chainloads to ARK4.BIN at 0x08D20000.
1.5. ARK4.BIN loads K.BIN to escalate into kernel mode.
- Alternatively you can directly load ARK4.BIN.

2. Code execution in Kernel Mode.
2.1. Inject rebootex to patch reboot buffer and trigger a reboot.
2.2. Rebootex injects cfw modules in boot sequence.
2.3. ARK's SystemControl (core CFW module) runs and sets up things.
2.4. ARK's CompatLayer loads to add device-specific patches and functionality.
2.5. Other CFW modules will load.




## Core CFW Modules.
1.1. SystemControl: main CFW module. Does most of the patching and sets things up for homebrew.
1.2. CompatLayer: adds extra functionality and fixes to allow ARK to run correctly in the installed environment.
1.3. Inferno: driver for games in ISO, CSO or ZSO format.
1.4. PopCorn: handles all the patching of POPS to allow custom PS1 games.
1.5. Stargate: anti DRM module.
1.6. VSHControl: allows the XMB to display and launch ISOs, load VSHMenu, custom PS1 and homebrew.




## Compatibility Layers

1. PSP Compatibility Layer.
1.2. Supports changing CPU frequency at will.
1.3. Support for high memory layout (not compatible with PSP 1K).
1.4. Supports USB Charging anywhere.
1.5. Fixed and patches specific to PSP.
1.6. FLASH0.ARK package installer.

2. PS Vita Compatibility Layer.
1.1. Filesystem and IO patches to better mimick PSP IO.
1.2. POPS Patches to allow loading of custom PEOPS SPU plugin. Deprecated.
1.3. FLASH0.ARK installation in ePSP flashram.
1.4. Vita Memory patches allowing to unlock extra RAM. Experimental.
1.5. Other Vita related patches and fixes.

3. PS Vita POPS (PS1) Compatibility Layer.
1.1. Partial support for PSP Homebrews.
1.2. Screen patches to visualize PSP VRAM output.
1.3. Audio patches to prevent crashing homebrews.
1.4. FLASH0.ARK installation in ePSP flashram.
1.5. Other Vita POPS related patches and fixes.




## Custom Menus
1.1. ARKMenu: custom launcher with extra functionality and better graphics.
1.2. PROVSH: simplified launcher with minimum functionality and graphics.
1.3. Recovery: custom recovery menu to handle CFW settings and manage plugins.
1.4. VSHMenu: standard VSH menu found on XMB.
1.5. xMenu: specially crafted Menu for Vita PS1 exploits.
1.6. gameMenu: in-game menu with special features. Coming soon.
