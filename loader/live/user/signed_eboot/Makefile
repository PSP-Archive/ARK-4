all:
	$(Q)make $@ -C  eboot $(OPT)
	$(Q)cp eboot/ARK.prx disc0/psp_game/sysdir/EBOOT.BIN
	$(Q)sign_np -elf eboot/ARK.elf EBOOT.BIN 0
	$(Q)$(ARKROOT)/contrib/PC/prxencrypter/prxencrypter eboot/ARK.prx
	$(Q)pack-pbp EBOOT.PBP eboot/PARAM.SFO eboot/ICON0.PNG NULL NULL NULL NULL data.psp NULL
	$(Q)mkisofs -o ark_loader.iso disc0/

clean:
	$(Q)make $@ -C eboot/sysmem $(OPT)
	$(Q)make $@ -C  eboot $(OPT)
	$(Q)rm -f EBOOT.BIN EBOOT.PBP data.psp ark_loader.iso
