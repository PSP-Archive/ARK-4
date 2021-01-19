all:
	$(Q)make $@ -C eboot/sysmem $(OPT)
	$(Q)make $@ -C  eboot $(OPT)
	$(Q)sign_np -elf eboot/ARK.elf EBOOT.BIN 00
	$(Q)mv EBOOT.BIN disc0/psp_game/sysdir/EBOOT.BIN
	$(Q)cp eboot/PARAM.SFO disc0/psp_game/param.sfo
	$(Q)$(PROVITA)/contrib/PC/prxencrypter/prxencrypter eboot/ARK.prx
	$(Q)pack-pbp EBOOT.PBP eboot/PARAM.SFO eboot/ICON0.PNG NULL NULL NULL NULL data.psp NULL
	#$(Q)$(PYTHON) make_iso.py

clean:
	$(Q)make $@ -C eboot/sysmem $(OPT)
	$(Q)make $@ -C  eboot $(OPT)
	$(Q)rm -f EBOOT.BIN EBOOT.PBP data.psp
