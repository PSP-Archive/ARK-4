# Number of Compilation Threads
OPT=-j8

PYTHON = $(shell which python2)
PROVITA ?= $(CURDIR)
K ?= psp660

export DEBUG PROVITA K

SUBDIRS = libs contrib/PC/prxencrypter core/systemctrl core/vitacompat core/vitapops core/pspcompat core/vshctrl core/stargate extras/menus/provsh extras/menus/arkMenu extras/menus/recovery core/popcorn core/inferno core/galaxy core/rebootex loader/stage2/live loader/stage2/compat loader/kxploit loader/stage1/linkless_payload loader/stage1/live_eboot contrib/PC/btcnf extras/menus/vshmenu
.PHONY: subdirs $(SUBDIRS) cleanobj clean cleanobj distclean copy-bin mkdir-dist encrypt-prx

all: subdirs mkdir-dist encrypt-prx copy-bin

copy-bin: loader/stage1/linkless_payload/h.bin loader/stage1/live_eboot/EBOOT.PBP loader/kxploit/k.bin contrib/PC/btcnf/psvbtinf.bin contrib/PC/btcnf/psvbtnnf.bin contrib/PC/btcnf/psvbtxnf.bin contrib/PSP/fake.cso extras/menus/arkMenu/EBOOT.PBP extras/menus/recovery/EBOOT.PBP extras/menus/vshmenu/satelite.prx
#	Common installation
	$(Q)cp loader/stage1/live_eboot/EBOOT.PBP dist/ARK_Live/EBOOT.PBP # Signed EBOOT
	$(Q)cp loader/kxploit/k.bin dist/ARK_Live/K.BIN # Kernel exploit for PSP
	$(Q)cp loader/vitabubble/PBOOT.PBP dist/VitaBubble/ # Vita 3.60 PBOOT.PBP bubble
	$(Q)cp loader/vitabubble/SAVEPATH.TXT dist/VitaBubble/ # Vita 3.60 PBOOT.PBP bubble
	$(Q)cp -r contrib/PSP/SAVEDATA/ARK_01234/ dist/ # ARK Savedata installation
	$(Q)cp loader/stage2/compat/ark.bin dist/ARK_01234/ARK.BIN # ARK-2 chainloader
	$(Q)cp loader/stage2/live/ark.bin dist/ARK_01234/ARK4.BIN # ARK-4 loader
	$(Q)cp loader/kxploit/vita360/K.BIN dist/ARK_01234/K.BIN # Kernel exploit for PS Vita 3.60 Henkaku
	$(Q)cp extras/menus/recovery/EBOOT.PBP dist/ARK_01234/RECOVERY.PBP # Default recovery menu
	$(Q)cp extras/menus/arkMenu/EBOOT.PBP dist/ARK_01234/MENU.PBP # Default launcher
	$(Q)cp extras/menus/arkMenu/DATA.PKG dist/ARK_01234/DATA.PKG # Launcher and Recovery resources
	$(Q)cp extras/menus/vshmenu/satelite.prx dist/ARK_01234/VSHMENU.PRX # Default vsh menu
	$(Q)cp loader/stage1/linkless_payload/h.bin dist/ARK_01234/H.BIN # game exploit loader
	$(Q)mv dist/FLASH0.ARK dist/ARK_01234/ # flash0 package
	
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
	$(Q)$(MAKE) $@ -C loader/stage2/compat
	$(Q)$(MAKE) $@ -C loader/kxploit
	$(Q)$(MAKE) $@ -C core/rebootex
	$(Q)$(MAKE) $@ -C core/systemctrl
	$(Q)$(MAKE) $@ -C core/vitacompat
	$(Q)$(MAKE) $@ -C core/vitapops
	$(Q)$(MAKE) $@ -C core/pspcompat
	$(Q)$(MAKE) $@ -C core/vshctrl
	$(Q)$(MAKE) $@ -C core/stargate
	$(Q)$(MAKE) $@ -C core/popcorn
	$(Q)$(MAKE) $@ -C core/inferno
	$(Q)$(MAKE) $@ -C core/galaxy
	$(Q)$(MAKE) $@ -C extras/menus/recovery
	$(Q)$(MAKE) $@ -C extras/menus/arkMenu
	$(Q)$(MAKE) $@ -C extras/menus/vshmenu
	$(Q)$(MAKE) $@ -C extras/menus/provsh
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
	$(Q)$(MAKE) $@ -C extras/menus/arkMenu

xmenu: libs
	$(Q)$(MAKE) $@ -C extras/menus/xMenu

recovery: libs
	$(Q)$(MAKE) $@ -C extras/menus/recovery

ark: rebootex

loader: ark

mkdir-dist:
	$(Q)mkdir dist | true
	$(Q)mkdir dist/VitaBubble | true
	$(Q)mkdir dist/ARK_Live | true

-include $(PROVITA)/.config
include $(PROVITA)/common/make/check.mak
include $(PROVITA)/common/make/quiet.mak
include $(PROVITA)/common/make/mod_enc.mak
