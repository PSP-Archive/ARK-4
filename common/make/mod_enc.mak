PYTHON=$(shell which python2)

dist/SYSCTRL.BIN: core/systemctrl/systemctrl.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< SystemControl 0x3007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/VITACOMP.BIN: core/vitacompat/vitacompat.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< VitaCompat 0x3007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/VITAPOPS.BIN: core/vitapops/vitapops.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< VitaPops 0x3007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)
	
dist/PSPCOMPAT.BIN: core/pspcompat/pspcompat.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< PSPCompat 0x3007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/VSHCTRL.BIN: core/vshctrl/vshctrl.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< VshCtrl 0x3007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/INFERNO.BIN: core/inferno/inferno.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< Inferno_Driver 0x1006
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/GALAXY.BIN: core/galaxy/galaxy.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< GalaxyController 0x1006
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/STARGATE.BIN: core/stargate/stargate.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< Stargate 0x1007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/POPCORN.BIN: core/popcorn/popcorn.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< PopcornManager 0x1006
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

##########
dist/POPSMAN.BIN: contrib/PSP/f0-kd-popsman.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< scePops_Manager 0x1007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/POPS.PRX: contrib/PSP/pops.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/UserModule.hdr $< pops 0x0000
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/PSPVMC.BIN: contrib/PSP/f0-vsh-module-libpspvmc.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/UserModule.hdr $< pspvmc_Library 0x0000
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/LIBFONTHV.BIN: contrib/PSP/libfont_hv.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/UserModule.hdr $< libFont_Library_HV 0x0000
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)
