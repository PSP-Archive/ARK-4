# Number of Compilation Threads
OPT=-j8

PYTHON = $(shell which python2)
PROVITA ?= $(CURDIR)

export DEBUG PROVITA

SUBDIRS = libs \
	contrib/PC/btcnf \
	contrib/PC/prxencrypter \
	core/rebootex \
	core/systemctrl \
	core/pspcompat \
	core/vitacompat \
	core/vitapops \
	core/inferno \
	core/stargate \
	core/popcorn \
	core/vshctrl \
	loader/stage1/linkless_payload \
	loader/stage1/live_eboot \
	loader/stage2/live \
	loader/stage2/compat \
	extras/menus/arkMenu \
	extras/menus/recovery \
	extras/menus/xMenu \
	extras/menus/vshmenu

.PHONY: subdirs $(SUBDIRS) cleanobj clean cleanobj copy-bin mkdir-dist encrypt-prx

all: subdirs kxploits mkdir-dist encrypt-prx copy-bin
	@echo "Build Done"

copy-bin: loader/stage1/linkless_payload/h.bin loader/stage1/live_eboot/EBOOT.PBP contrib/PC/btcnf/psvbtinf.bin contrib/PC/btcnf/psvbtnnf.bin contrib/PC/btcnf/psvbtxnf.bin contrib/PSP/fake.cso extras/menus/arkMenu/EBOOT.PBP extras/menus/recovery/EBOOT.PBP extras/menus/xMenu/EBOOT.PBP extras/menus/vshmenu/satelite.prx
#	Common installation
	$(Q)cp loader/stage1/live_eboot/EBOOT.PBP dist/ARK_Live/EBOOT.PBP # Signed EBOOT
	$(Q)cp loader/kxploit/psp660/k.bin dist/ARK_Live/K.BIN # Kernel exploit for PSP
	$(Q)cp loader/stage1/vitabubble/PBOOT.PBP dist/VitaBubble/ # Vita 3.60 PBOOT.PBP bubble
	$(Q)cp loader/stage1/vitabubble/SAVEPATH.TXT dist/VitaBubble/ # Vita 3.60 PBOOT.PBP bubble
	$(Q)cp -r contrib/PSP/SAVEDATA/ARK_01234/ dist/ # ARK Savedata installation
	$(Q)cp loader/stage2/compat/ark.bin dist/ARK_01234/ARK.BIN # ARK-2 chainloader
	$(Q)cp loader/stage2/live/ark.bin dist/ARK_01234/ARK4.BIN # ARK-4 loader
	$(Q)cp loader/kxploit/vita360/k.bin dist/ARK_01234/K.BIN # Kernel exploit for PS Vita 3.60 Henkaku
	$(Q)cp extras/menus/recovery/EBOOT.PBP dist/ARK_01234/RECOVERY.PBP # Default recovery menu
	$(Q)cp extras/menus/arkMenu/EBOOT.PBP dist/ARK_01234/MENU.PBP # Default launcher
	$(Q)cp extras/menus/xMenu/EBOOT.PBP dist/ARK_01234/XMENU.PBP # PS1 launcher
	$(Q)cp extras/menus/arkMenu/themes/classic/DATA.PKG dist/ARK_01234/DATA.PKG # Launcher and Recovery resources
	$(Q)cp extras/menus/vshmenu/satelite.prx dist/ARK_01234/VSHMENU.PRX # Default vsh menu
	$(Q)cp loader/stage1/linkless_payload/h.bin dist/ARK_01234/H.BIN # game exploit loader
	$(Q)mv dist/FLASH0.ARK dist/ARK_01234/ # flash0 package
	
encrypt-prx: \
	dist/SYSCTRL.BIN dist/VITACOMP.BIN dist/VITAPOPS.BIN dist/PSPCOMPAT.BIN dist/VSHCTRL.BIN dist/INFERNO.BIN dist/STARGATE.BIN dist/POPCORN.BIN
	$(Q)cp contrib/PC/btcnf/psvbtinf.bin dist/PSVBTINF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtnnf.bin dist/PSVBTNNF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtxnf.bin dist/PSVBTXNF.BIN
	$(Q)$(PYTHON) contrib/PC/pack/pack.py -p dist/FLASH0.ARK contrib/PC/pack/packlist.txt


kxploits:
	$(Q)$(MAKE) $@ K=psp660 -C loader/kxploit
	$(Q)$(MAKE) $@ K=vita320 -C loader/kxploit
	$(Q)$(MAKE) $@ K=vita360 -C loader/kxploit
	$(Q)$(MAKE) $@ K=vita370 -C loader/kxploit

# Only clean non-library code
cleanobj:
	$(Q)$(MAKE) clean CLEANOBJ=1

clean:
	$(Q)$(MAKE) $@ -C libs
	$(Q)$(MAKE) $@ -C loader/stage1/linkless_payload
	$(Q)$(MAKE) $@ -C loader/stage1/live_eboot
	$(Q)$(MAKE) $@ -C loader/stage2/live
	$(Q)$(MAKE) $@ -C loader/stage2/compat
	$(Q)$(MAKE) $@ -C core/rebootex
	$(Q)$(MAKE) $@ -C core/systemctrl
	$(Q)$(MAKE) $@ -C core/vitacompat
	$(Q)$(MAKE) $@ -C core/vitapops
	$(Q)$(MAKE) $@ -C core/pspcompat
	$(Q)$(MAKE) $@ -C core/vshctrl
	$(Q)$(MAKE) $@ -C core/stargate
	$(Q)$(MAKE) $@ -C core/popcorn
	$(Q)$(MAKE) $@ -C core/inferno
	$(Q)$(MAKE) $@ -C extras/menus/recovery
	$(Q)$(MAKE) $@ -C extras/menus/arkMenu
	$(Q)$(MAKE) $@ -C extras/menus/vshmenu
	$(Q)$(MAKE) $@ -C extras/menus/provsh
	$(Q)$(MAKE) $@ K=psp660 -C loader/kxploit
	$(Q)$(MAKE) $@ K=vita360 -C loader/kxploit
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
include $(PROVITA)/common/make/quiet.mak
include $(PROVITA)/common/make/mod_enc.mak
