# General CFW Issues
- ???

# PSP Issues
- ???

# Vita Standalone
- PS1 games don't work (there are two ways to fix this and each of them is not fully explored yet).
- High memory support doesn't work due to that area of RAM being used to load flash files.
- There is no native Vita-side patching done, so there's some limitations imposed by Official Sony pspemu (can't create folders in /PSP/GAME/, can't create files named EBOOT.PBP, etc), some workarounds include being able to use /PSP/APPS/ and /PSP/VHBL/ folders for homebrews and alternative names for eboots (FBOOT.PBP, VBOOT.PBP, wmenu.bin).
- The Leda plugin conflicts with rebootex runtime module loading, shouldn't be much of an issue since no homebrew or plugin uses rebootex runtime module loading on Vita.

# Vita Adrenaline
- ???

# Launcher Issues
- Some random crashes in the game menu (most likely threading issues).
- FTP Client is generally unstable.

# Plugins Issues
- ???

