# CFW Issues
- PS1 games don't work on PS Vita (there are two ways to fix this and each of them is not fully explored yet).
- There is no kernel exploit available for PS Vita 3.65+.
- There is no native Vita-side patching done, so there's some limitations imposed by Official Sony pspemu (can't create folders in /PSP/GAME/, can't create files named EBOOT.PBP, etc), some workarounds include being able to use /PSP/APPS/ and /PSP/VHBL/ folders for homebrews and alternative names for eboots (FBOOT.PBP, VBOOT.PBP, wmenu.bin).
- On PS Vita, the Leda plugin conflicts with rebootex runtime module loading, shouldn't be much of an issue since no homebrew or plugin uses rebootex runtime module loading on Vita.

# Launcher Issues
- Incompatible with non-ASCII characters.


# Plugins Issues
