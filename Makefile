# Number of Compilation Threads
OPT=-j8

PYTHON = $(shell which python2)
PROVITA ?= $(CURDIR)
K ?= psp660

export DEBUG PROVITA K

SUBDIRS = libs contrib/PC/prxencrypter core/systemctrl core/vitacompat core/vitapops core/pspcompat core/vshctrl core/stargate menu/provsh core/popcorn core/inferno core/galaxy core/rebootex loader/stage2/live loader/kxploit loader/stage1/linkless_payload loader/stage1/live_eboot contrib/PC/btcnf extras/vshmenu/classic
.PHONY: subdirs $(SUBDIRS) cleanobj clean cleanobj distclean copy-bin mkdir-dist encrypt-prx

all: subdirs mkdir-dist encrypt-prx copy-bin

copy-bin: loader/stage1/linkless_payload/h.bin loader/stage1/live_eboot/EBOOT.PBP loader/kxploit/k.bin contrib/PC/btcnf/psvbtinf.bin contrib/PC/btcnf/psvbtnnf.bin contrib/PC/btcnf/psvbtxnf.bin contrib/PSP/fake.cso menu/provsh/EBOOT.PBP extras/vshmenu/classic/satelite.prx
#	Common installation
	$(Q)cp loader/stage1/linkless_payload/h.bin dist/H.BIN # game exploit loader
	$(Q)cp loader/stage1/live_eboot/EBOOT.PBP dist/EBOOT.PBP # Signed EBOOT
	$(Q)cp loader/stage1/live_eboot/eboot/EBOOT.PBP dist/WMENU.BIN # Unsigned EBOOT for VHBL
	$(Q)cp loader/stage2/live/ark.bin dist/ARK.BIN # ARK installer and loader
	$(Q)cp loader/kxploit/k.bin dist/K.BIN # Kernel exploit
	$(Q)cp menu/provsh/EBOOT.PBP dist/MENU.PBP # Default menu
	$(Q)cp extras/vshmenu/classic/satelite.prx dist/VSHMENU.PRX # Default vsh menu
	
encrypt-prx: \
	dist/SYSCTRL0.BIN dist/VITACOMP.BIN dist/VITAPOPS.BIN dist/PSPCOMPAT.BIN dist/VSHCTRL.BIN dist/INFERNO0.BIN dist/GALAXY00.BIN dist/STARGATE.BIN dist/POPCORN0.BIN \
	dist/POPSMAN0.BIN dist/POPS.PRX dist/PSPVMC00.BIN dist/MEDIASYN.BIN dist/MODULEMR.BIN dist/NPDRM.PRX dist/NP966000.BIN
	$(Q)cp contrib/PC/btcnf/psvbtinf.bin dist/PSVBTINF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtinf.bin dist/PSVBTNNF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtxnf.bin dist/PSVBTXNF.BIN
	$(Q)cp contrib/PSP/fake.cso dist/FAKECSO0.BIN
	$(Q)$(PYTHON) contrib/PC/pack/pack.py -p dist/FLASH0.ARK contrib/PC/pack/packlist.txt


# Only clean non-library code
cleanobj:
	$(Q)$(MAKE) clean CLEANOBJ=1

distclean clean:
ifndef CLEANOBJ
	$(Q)$(MAKE) $@ -C libs
endif
	$(Q)$(MAKE) $@ -C loader/stage1/linkless_payload
	$(Q)$(MAKE) $@ -C loader/stage1/live_eboot
	$(Q)$(MAKE) $@ -C loader/stage2/live
	$(Q)$(MAKE) $@ -C loader/kxploit
	$(Q)$(MAKE) $@ -C core/rebootex
	$(Q)$(MAKE) $@ -C core/systemctrl
	$(Q)$(MAKE) $@ -C core/vitacompat
	$(Q)$(MAKE) $@ -C core/vitapops
	$(Q)$(MAKE) $@ -C core/pspcompat
	$(Q)$(MAKE) $@ -C core/stargate
	$(Q)$(MAKE) $@ -C core/popcorn
	$(Q)$(MAKE) $@ -C core/inferno
	$(Q)$(MAKE) $@ -C core/galaxy
	$(Q)$(MAKE) $@ -C menu/provsh
	$(Q)$(MAKE) $@ -C extras/vshmenu/classic/
	$(Q)-rm -rf dist *~ | true
	$(Q)-rm -f contrib/PC/btcnf/psvbtinf.bin
	$(Q)-rm -f contrib/PC/btcnf/psvbtnnf.bin
	$(Q)-rm -f contrib/PC/btcnf/psvbtxnf.bin
	$(Q)$(PYTHON) cleandeps.py

subdirs: $(SUBDIRS)

$(filter-out libs, $(SUBDIRS)): libs
	$(Q)$(MAKE) $(OPT) -C $@

libs:
	$(Q)$(MAKE) $(OPT) -C $@

arkmenu: libs
	$(Q)$(MAKE) $@ -C menu/arkMenu

xmenu: libs
	$(Q)$(MAKE) $@ -C menu/xMenu

ark: rebootex

loader: ark

mkdir-dist:
	$(Q)mkdir dist | true

-include $(PROVITA)/.config
include $(PROVITA)/common/make/check.mak
include $(PROVITA)/common/make/quiet.mak
include $(PROVITA)/common/make/mod_enc.mak
