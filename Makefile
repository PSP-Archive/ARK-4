# Number of Compilation Threads
OPT=-j8

PYTHON = $(shell which python2)
ARKROOT ?= $(CURDIR)

export DEBUG ARKROOT

SUBDIRS = libs \
	contrib/PC/btcnf \
	contrib/PC/prxencrypter \
	core/systemctrl \
	core/inferno \
	core/stargate \
	core/popcorn \
	core/vshctrl \
	core/compat/psp/rebootex \
	core/compat/psp \
	core/compat/vita/rebootex \
	core/compat/vita \
	core/compat/vitapops/rebootex \
	core/compat/vitapops \
	core/compat/pentazemin/rebootex \
	core/compat/pentazemin \
	loader/live/user/linkless_payload \
	loader/live/user/signed_eboot \
	loader/live/kernel/kernel_loader \
	loader/live/kernel/chain_loader \
	loader/live/kernel/kram_dumper \
	loader/live/kernel/idstorage_dumper \
	loader/live/kernel/psp_flash_dumper \
	loader/live/kernel/vita_flash_dumper \
	loader/perma/cipl/payloadex \
	loader/perma/cipl/mainbinex \
	loader/perma/cipl/combine \
	loader/perma/cipl/installer \
	loader/perma/cipl/installer/kpspident \
	loader/dc/dcman \
	loader/dc/msipl/payloadex \
	loader/dc/msipl/mainbinex \
	loader/dc/tmctrl/rebootex \
	loader/dc/tmctrl \
	loader/dc/vunbricker \
	loader/dc/installer \
	extras/menus/arkMenu \
	extras/menus/recovery \
	extras/menus/xMenu \
	extras/menus/advancedvsh \
	extras/xmbctrl \
	extras/usbdevice \
	extras/idsregeneration

.PHONY: subdirs $(SUBDIRS) cleanobj clean cleanobj copy-bin mkdir-dist encrypt-prx

all: subdirs kxploits mkdir-dist encrypt-prx copy-bin
	@echo "Build Done"

copy-bin:
#	Common installation
	$(Q)cp loader/live/user/signed_eboot/EBOOT.PBP dist/ARK_Live/EBOOT.PBP # Signed EBOOT
	$(Q)cp loader/live/kernel/kxploit/psp660/K.BIN dist/ARK_Live/K.BIN # Kernel exploit for PSP
	$(Q)cp loader/live/user/vitabubble/PBOOT.PBP dist/Vita/Standalone/ # Vita 3.60 PBOOT.PBP bubble
	$(Q)cp loader/live/kernel/kxploit/vita360/K.BIN dist/Vita/Standalone/K.BIN # Kernel exploit for Vita 3.60+
	$(Q)cp loader/live/kernel/kxploit/cfw/K.BIN dist/Vita/Adrenaline/K.BIN # kxploit for CFW
	$(Q)cp loader/perma/infinity/EBOOT.PBP dist/Infinity/ # Infinity with ARK support
	$(Q)cp loader/perma/infinity/EBOOT_GO.PBP dist/Infinity/ # Infinity with ARK support (PSP Go)
	$(Q)cp -r contrib/PSP/SAVEDATA/ARK_01234/ dist/ # ARK Savedata installation
	$(Q)cp loader/live/kernel/chain_loader/ARK.BIN dist/ARK_01234/ARK.BIN # ARK-2 chainloader
	$(Q)cp loader/live/kernel/kernel_loader/ARK4.BIN dist/ARK_01234/ARK4.BIN # ARK-4 loader
	$(Q)cp loader/live/kernel/kxploit/dummy/K.BIN dist/ARK_01234/K.BIN # Dummy Kernel exploit
	$(Q)cp loader/live/user/linkless_payload/H.BIN dist/ARK_01234/H.BIN # game exploit loader
	$(Q)cp -r contrib/PSP/GAME/ARK_DC/ dist/ # ARK DC installer
	$(Q)cp loader/dc/installer/EBOOT.PBP dist/ARK_DC/ # ARK DC installer
	$(Q)cp loader/perma/cipl/installer/EBOOT.PBP dist/ARK_cIPL/EBOOT.PBP
	$(Q)cp loader/perma/cipl/installer/kpspident/kpspident.prx dist/ARK_cIPL/kpspident.prx
	$(Q)cp contrib/PSP/GAME/ARK_cIPL/ipl_update.prx dist/ARK_cIPL/ipl_update.prx
	$(Q)cp extras/menus/recovery/EBOOT.PBP dist/ARK_01234/RECOVERY.PBP # Default recovery menu
	$(Q)cp extras/menus/arkMenu/EBOOT.PBP dist/ARK_01234/VBOOT.PBP # Default launcher
	$(Q)cp extras/menus/xMenu/EBOOT.PBP dist/ARK_01234/XBOOT.PBP # PS1 launcher
	$(Q)cp extras/menus/arkMenu/themes/ARK_Revamped/THEME.ARK dist/ARK_01234/THEME.ARK # Launcher and Recovery resources
	$(Q)cp extras/xmbctrl/xmbctrl.prx dist/ARK_01234/XMBCTRL.PRX # XMB Control Module
	$(Q)cp extras/xmbctrl/translations/XMB_*.TXT dist/ARK_01234/ # XMB Control translation files
	$(Q)cp extras/idsregeneration/idsregeneration.prx dist/ARK_01234/IDSREG.PRX # idsregeneration
	$(Q)cp extras/usbdevice/usbdevice.prx dist/ARK_01234/USBDEV.PRX # USB Device Driver
	$(Q)cp extras/menus/advancedvsh/satelite.prx dist/ARK_01234/VSHMENU.PRX # New Default & Advanced VSH Menu
	$(Q)cp -r extras/menus/arkMenu/themes dist/
	$(Q)cp contrib/README.TXT dist/
	$(Q)mv dist/FLASH0.ARK dist/ARK_01234/ # flash0 package
	$(Q)cp -r dist/ARK_01234 dist/ARK_DC/
	
encrypt-prx: \
	dist/SYSCTRL.BIN \
	dist/VSHCTRL.BIN \
	dist/INFERNO.BIN \
	dist/STARGATE.BIN \
	dist/POPCORN.BIN \
	dist/PSPCOMP.BIN \
	dist/VITACOMP.BIN \
	dist/VITAPOPS.BIN \
	dist/VITAPLUS.BIN
	$(Q)cp contrib/PC/btcnf/psvbtinf.bin dist/PSVBTINF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtcnf.bin dist/PSVBTCNF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtxnf.bin dist/PSVBTXNF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtjnf.bin dist/PSVBTJNF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtknf.bin dist/PSVBTKNF.BIN
	$(Q)$(PYTHON) contrib/PC/pack/pack.py -p dist/FLASH0.ARK contrib/PC/pack/packlist.txt


kxploits:
	$(Q)$(MAKE) $@ K=dummy -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=psp660 -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=vita320 -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=vita360 -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=cfw -C loader/live/kernel/kxploit

# Only clean non-library code
cleanobj:
	$(Q)$(MAKE) clean CLEANOBJ=1

clean:
	$(Q)$(MAKE) $@ -C libs
	$(Q)$(MAKE) $@ -C loader/live/user/linkless_payload
	$(Q)$(MAKE) $@ -C loader/live/user/signed_eboot
	$(Q)$(MAKE) $@ -C loader/live/kernel/kernel_loader
	$(Q)$(MAKE) $@ -C loader/live/kernel/chain_loader
	$(Q)$(MAKE) $@ -C loader/live/kernel/kram_dumper
	$(Q)$(MAKE) $@ -C loader/live/kernel/idstorage_dumper
	$(Q)$(MAKE) $@ -C loader/live/kernel/psp_flash_dumper
	$(Q)$(MAKE) $@ -C loader/live/kernel/vita_flash_dumper
	$(Q)$(MAKE) $@ -C core/compat/psp/rebootex
	$(Q)$(MAKE) $@ -C core/compat/vita/rebootex
	$(Q)$(MAKE) $@ -C core/compat/vitapops/rebootex
	$(Q)$(MAKE) $@ -C core/compat/pentazemin/rebootex
	$(Q)$(MAKE) $@ -C core/systemctrl
	$(Q)$(MAKE) $@ -C core/vshctrl
	$(Q)$(MAKE) $@ -C core/stargate
	$(Q)$(MAKE) $@ -C core/popcorn
	$(Q)$(MAKE) $@ -C core/inferno
	$(Q)$(MAKE) $@ -C core/compat/psp
	$(Q)$(MAKE) $@ -C core/compat/vita
	$(Q)$(MAKE) $@ -C core/compat/vitapops
	$(Q)$(MAKE) $@ -C core/compat/pentazemin
	$(Q)$(MAKE) $@ -C extras/menus/recovery
	$(Q)$(MAKE) $@ -C extras/menus/arkMenu
	$(Q)$(MAKE) $@ -C extras/menus/advancedvsh
	$(Q)$(MAKE) $@ -C extras/menus/provsh
	$(Q)$(MAKE) $@ -C extras/menus/xMenu
	$(Q)$(MAKE) $@ -C extras/xmbctrl
	$(Q)$(MAKE) $@ -C extras/usbdevice
	$(Q)$(MAKE) $@ -C extras/idsregeneration
	$(Q)$(MAKE) $@ K=dummy -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=psp660 -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=vita320 -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=vita360 -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=cfw -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ -C contrib/PC/btcnf/
	$(Q)$(MAKE) $@ -C loader/perma/cipl/payloadex
	$(Q)$(MAKE) $@ -C loader/perma/cipl/mainbinex
	$(Q)$(MAKE) $@ -C loader/perma/cipl/combine
	$(Q)$(MAKE) $@ -C loader/perma/cipl/installer
	$(Q)$(MAKE) $@ -C loader/perma/cipl/installer/kpspident
	$(Q)$(MAKE) $@ -C loader/dc/dcman
	$(Q)$(MAKE) $@ -C loader/dc/installer
	$(Q)$(MAKE) $@ -C loader/dc/msipl/mainbinex
	$(Q)$(MAKE) $@ -C loader/dc/msipl/payloadex
	$(Q)$(MAKE) $@ -C loader/dc/tmctrl/rebootex
	$(Q)$(MAKE) $@ -C loader/dc/tmctrl
	$(Q)$(MAKE) $@ -C loader/dc/vunbricker
	$(Q)-rm -rf dist *~ | true
	$(Q)-rm -rf common/utils/*.o
	$(Q)$(PYTHON) contrib/PC/scripts/cleandeps.py

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

mkdir-dist:
	$(Q)mkdir dist | true
	$(Q)mkdir dist/Vita | true
	$(Q)mkdir dist/ARK_Live | true
	$(Q)mkdir dist/Infinity | true
	$(Q)mkdir dist/ARK_DC | true
	$(Q)mkdir dist/ARK_cIPL | true
	$(Q)mkdir dist/Vita/Adrenaline | true
	$(Q)mkdir dist/Vita/Standalone | true

-include $(ARKROOT)/.config
include $(ARKROOT)/common/make/quiet.mak
include $(ARKROOT)/common/make/mod_enc.mak
