PYTHON=$(shell which python2)

dist/SYSCTRL.BIN: core/systemctrl/systemctrl.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< SystemControl 0x3007
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

dist/PSPCOMP.BIN: core/compat/psp/pspcompat.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< ARKPSPCompat 0x3007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/VITACOMP.BIN: core/compat/vita/vitacompat.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< ARKVitaCompat 0x3007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/VITAPOPS.BIN: core/compat/vitapops/vitapops.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< ARKVitaPopsCompat 0x3007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/POPCORNV.BIN: core/compat/vita/popcorn/popcorn.prx
	$(Q)psp-fixup-imports -m ./common/nidmap.txt $<
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< PopcornManager 0x1006
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/POPSMAN.BIN: contrib/PSP/popsman.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< scePops_Manager 0x1007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/PSPVMC.BIN: contrib/PSP/libpspvmc.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/UserModule.hdr $< pspvmc_Library 0x0000
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/MEDIASYNC.BIN: contrib/PSP/mediasync.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< sceMediaSync 0x1006
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/MODMAN.BIN: contrib/PSP/modulemgr.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< sceModuleManager 0x1006
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/NP966000.BIN: contrib/PSP/np9660.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< sceNp9660_driver 0x1006
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/INTERRUP.BIN: contrib/PSP/interruptman.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< sceInterruptManager 0x1006
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/LIBFONTHV.BIN: contrib/PSP/libfont_hv.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/UserModule.hdr $< libFont_Library_HV 0x0000
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)

dist/IMPOSE.BIN: contrib/PSP/impose_05g.prx
	$(Q)$(PYTHON) ./contrib/PC/pspgz/pspgz.py $(patsubst %.prx,%.gz.prx,$<) contrib/PC/pspgz/SystemControl.hdr $< sceImpose_Driver 0x1007
	$(Q)cp $(patsubst %.prx,%.gz.prx,$<) $@
	$(Q)rm -f $(patsubst %.prx,%.gz.prx,$<) $(patsubst %.prx,%.enc.prx,$<)
