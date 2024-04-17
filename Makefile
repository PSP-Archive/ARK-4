# Number of Compilation Threads
OPT=-j$(shell nproc)

PYTHON = $(shell which python3)
ARKROOT ?= $(CURDIR)

export DEBUG ARKROOT

SUBDIRS = libs \
	contrib/PC/prxencrypter \
	core/systemctrl \
	core/inferno \
	core/stargate \
	core/popcorn \
	core/vshctrl \
	core/compat/psp/rebootex \
	core/compat/psp/btcnf \
	core/compat/psp \
	core/compat/vita/rebootex \
	core/compat/vita/btcnf \
	core/compat/vita \
	core/compat/vitapops/rebootex \
	core/compat/vitapops/btcnf \
	core/compat/vitapops \
	core/compat/pentazemin/rebootex \
	core/compat/pentazemin/btcnf \
	core/compat/pentazemin \
	extras/modules/iop \
	extras/modules/ipl_update \
	extras/modules/kpspident \
	extras/modules/peops \
	extras/modules/xmbctrl \
	extras/modules/usbdevice \
	extras/modules/idsregeneration \
	extras/modules/kbooti_update \
	loader/live/user/linkless_payload \
	loader/live/user/signed_eboot \
	loader/live/user/psxloader \
	loader/live/kernel/kernel_loader \
	loader/live/kernel/psxloader \
	loader/live/kernel/chain_loader \
	loader/live/kernel/kram_dumper \
	loader/live/kernel/idstorage_dumper \
	loader/live/kernel/psp_flash_dumper \
	loader/live/kernel/vita_flash_dumper \
	loader/live/kernel/pandorizer \
	loader/rebootex/ms_payloadex \
	loader/rebootex/nand_payloadex \
	loader/perma/cipl/classic/mainbinex \
	loader/perma/cipl/classic/combine \
	loader/dc/dcman \
	loader/dc/msipl/newipl/loader \
	loader/dc/msipl/mainbinex \
	loader/dc/tmctrl/rebootex \
	loader/dc/tmctrl \
	loader/dc/vunbricker \
	extras/menus/arkMenu \
	extras/menus/recovery \
	extras/menus/xMenu \
	extras/menus/vshmenu \
	extras/apps/installer \
	extras/apps/uninstaller 

.PHONY: subdirs $(SUBDIRS) cleanobj clean cleanobj copy-bin mkdir-dist encrypt-prx copy-dcark pack-flash0

all: subdirs cipl msipl kxploits mkdir-dist encrypt-prx copy-dcark pack-flash0 copy-bin
	@echo "Build Done"

#	Common installation
copy-bin:
	$(Q)cp -r contrib/PSP/SAVEDATA/ARK_01234/ dist/ # ARK Savedata installation
	$(Q)cp -r contrib/PSP/GAME/ARK_DC/ dist/PSP/ # ARK DC installer
	$(Q)cp loader/dc/installer/EBOOT.PBP dist/PSP/ARK_DC/ # ARK DC installer
	$(Q)cp loader/vpk/bin/psp/PBOOT.PBP dist/PSVita/Standalone/
	$(Q)cp loader/perma/cipl/installer/EBOOT.PBP dist/PSP/ARK_cIPL/EBOOT.PBP
	$(Q)cp loader/perma/infinity/EBOOT.PBP dist/PSP/Infinity/ # Infinity with ARK support
	$(Q)cp loader/perma/infinity/EBOOT_GO.PBP dist/PSP/Infinity/ # Infinity with ARK support (PSP Go)
	$(Q)cp loader/live/user/linkless_payload/H.BIN dist/ARK_01234/H.BIN # game exploit loader
	$(Q)cp loader/live/user/signed_eboot/EBOOT.PBP dist/ARK_Loader/EBOOT.PBP # Signed EBOOT
	$(Q)cp loader/live/user/signed_eboot/ark_loader.iso dist/PSVita/Standalone/
	$(Q)cp loader/live/user/psxloader/EBOOT.PBP dist/PSVita/PS1CFW/SCPS10084/
	$(Q)cp loader/live/user/psxloader/psxloader.prx dist/PSVita/PS1CFW/
	$(Q)cp loader/live/kernel/chain_loader/ARK.BIN dist/ARK_01234/ARK.BIN # ARK-2 chainloader
	$(Q)cp loader/live/kernel/kernel_loader/ARK4.BIN dist/ARK_01234/ARK4.BIN # ARK-4 loader
	$(Q)cp loader/live/kernel/psxloader/ARKX.BIN dist/ARK_01234/ARKX.BIN # ARK-X loader
	$(Q)cp loader/live/kernel/psxloader/ps1cfw_enabler/ps1cfw_enabler.suprx dist/PSVita/PS1CFW/
	$(Q)cp loader/live/kernel/kxploit/sceUID/K.BIN dist/ARK_01234/K.BIN # Kernel exploit for PSP 6.6X and Vita 3.60+
	$(Q)cp loader/live/kernel/kxploit/sceSdGetLastIndex/K.BIN dist/ARK_Loader/K.BIN # Alternative Kernel exploit for PSP 6.6X
	$(Q)cp extras/modules/kpspident/kpspident.prx dist/PSP/ARK_cIPL/kpspident.prx
	$(Q)cp extras/modules/ipl_update/ipl_update.prx dist/PSP/ARK_cIPL/ipl_update.prx
	$(Q)cp extras/modules/kbooti_update/kbooti_update.prx dist/PSP/ARK_cIPL/kbooti_update.prx
	$(Q)cp extras/modules/xmbctrl/xmbctrl.prx dist/ARK_01234/XMBCTRL.PRX # XMB Control Module
	$(Q)cp extras/modules/idsregeneration/idsregeneration.prx dist/ARK_01234/IDSREG.PRX # idsregeneration
	$(Q)cp extras/modules/usbdevice/usbdevice.prx dist/ARK_01234/USBDEV.PRX # USB Device Driver
	$(Q)cp extras/modules/peops/peops.prx dist/ARK_01234/PS1SPU.PRX
	$(Q)cp extras/menus/recovery/ark_recovery.prx dist/ARK_01234/RECOVERY.PRX # Default recovery menu
	$(Q)cp extras/menus/arkMenu/EBOOT.PBP dist/ARK_01234/VBOOT.PBP # Default launcher
	$(Q)cp extras/menus/arkMenu/LANG.ARK dist/ARK_01234/LANG.ARK # Translations
	$(Q)cp extras/menus/xMenu/EBOOT.PBP dist/ARK_01234/XBOOT.PBP # PS1 launcher
	$(Q)cp extras/menus/arkMenu/themes/ARK_Revamped/THEME.ARK dist/ARK_01234/THEME.ARK # Launcher resources
	$(Q)cp extras/menus/vshmenu/satelite.prx dist/ARK_01234/VSHMENU.PRX # New Default & Advanced VSH Menu
	$(Q)cp extras/apps/installer/EBOOT.PBP dist/PSP/ARK_Full_Installer # Full installer
	$(Q)cp extras/apps/uninstaller/EBOOT.PBP dist/PSP/ARK_Uninstaller # ARK-4 Uninstaller
	$(Q)cp contrib/UPDATER.TXT dist/ARK_01234/
	$(Q)cp contrib/SETTINGS.TXT dist/ARK_01234/
	$(Q)cp contrib/PSP/mediasync.prx dist/ARK_01234/MEDIASYN.PRX
	$(Q)cp contrib/PSP/popsman.prx dist/ARK_01234/POPSMAN.PRX
	$(Q)cp contrib/PSP/pops_01g.prx dist/ARK_01234/POPS.PRX
	$(Q)cp core/compat/psp/btcnf/pstbtcnf_tt.bin dist/PSP/Pops4Tool/TT/pstbtcnf.bin
	$(Q)cp core/compat/psp/btcnf/pstbtcnf_dt.bin dist/PSP/Pops4Tool/DT/pstbtcnf.bin
	$(Q)cp contrib/PSP/pops_01g.prx dist/PSP/Pops4Tool/kd/
	$(Q)cp contrib/PSP/popsman.prx dist/PSP/Pops4Tool/kd/
	$(Q)cp contrib/PSP/libpspvmc.prx dist/PSP/Pops4Tool/vsh/module/
	$(Q)cp -r extras/menus/arkMenu/themes dist/
	$(Q)rm -rf dist/themes/translations
	$(Q)cp contrib/README.TXT dist/
	$(Q)mv dist/FLASH0.ARK dist/ARK_01234/ # flash0 package
	$(Q)cp -r dist/ARK_01234 dist/PSP/ARK_DC/
	$(Q)cp -r dist/ARK_01234 dist/PC/MagicMemoryCreator/TM/DCARK/
	$(Q)find dist/themes/ -type d -name 'resources' -exec rm -rf {} \; 2>/dev/null || true
	$(q)mkdir -p loader/vpk/bin/save/ARK_01234
	$(Q)cp -r dist/ARK_01234 loader/vpk/bin/save/
	$(Q)cp loader/live/kernel/psxloader/ps1cfw_enabler/ps1cfw_enabler.suprx loader/vpk/bin/psx/
	$(Q)cd loader/vpk/bin/ && zip -r ../../../dist/PSVita/FasterARK.vpk * && cd $(ARKROOT)
	$(Q)$(MAKE) -C extras/apps/updater/
	$(Q)cp extras/apps/updater/EBOOT_PSP.PBP dist/UPDATE/EBOOT.PBP
	$(Q)cp loader/dc/msipl/newipl/msipl_*.bin dist/PC/MagicMemoryCreator/TM/DCARK/
	$(Q)cp loader/dc/btcnf/pspbtcnf*_dc.bin dist/PC/MagicMemoryCreator/TM/DCARK/kd/
	$(Q)cp contrib/PSP/IPL/tm_mloader.bin dist/PC/MagicMemoryCreator/TM/DCARK/
	$(Q)cp loader/dc/msipl/newipl/loader/ipl.bin dist/PC/MagicMemoryCreator/msipl.bin
	$(Q)cp loader/dc/tmctrl/tmctrl.prx dist/PC/MagicMemoryCreator/TM/DCARK/
	$(Q)cp loader/dc/dcman/dcman.prx dist/PC/MagicMemoryCreator/TM/DCARK/kd/
	$(Q)cp extras/modules/iop/iop.prx dist/PC/MagicMemoryCreator/TM/DCARK/kd/
	$(Q)cp loader/dc/vunbricker/resurrection.prx dist/PC/MagicMemoryCreator/TM/DCARK/vsh/module/
	$(Q)cp extras/modules/ipl_update/ipl_update.prx dist/PC/MagicMemoryCreator/TM/DCARK/kd/
	$(Q)cp loader/dc/msipl/newipl/loader/msipl.bin dist/PC/MagicMemoryCreator/TM/DCARK/msipl.raw
	$(Q)cp contrib/PSP/IPL/nandipl_01G.bin dist/PC/MagicMemoryCreator/TM/DCARK/ipl_01g.bin
	$(Q)cp contrib/PSP/IPL/nandipl_02G.bin dist/PC/MagicMemoryCreator/TM/DCARK/ipl_02g.bin
	$(Q)cp contrib/PSP/IPL/nandipl_03G.bin dist/PC/MagicMemoryCreator/TM/DCARK/ipl_03g.bin
	$(Q)cp contrib/PSP/IPL/nandipl_04G.bin dist/PC/MagicMemoryCreator/TM/DCARK/ipl_04g.bin
	$(Q)cp contrib/PSP/IPL/nandipl_05G.bin dist/PC/MagicMemoryCreator/TM/DCARK/ipl_05g.bin
	$(Q)cp contrib/PSP/IPL/nandipl_07G.bin dist/PC/MagicMemoryCreator/TM/DCARK/ipl_07g.bin
	$(Q)cp contrib/PSP/IPL/nandipl_09G.bin dist/PC/MagicMemoryCreator/TM/DCARK/ipl_09g.bin
	$(Q)cp contrib/PSP/IPL/nandipl_11G.bin dist/PC/MagicMemoryCreator/TM/DCARK/ipl_11g.bin
	$(Q)cp loader/perma/cipl/new/ipl_01G.dec dist/PC/MagicMemoryCreator/TM/DCARK/nandipl_01g.bin
	$(Q)cp loader/perma/cipl/new/ipl_02G.dec dist/PC/MagicMemoryCreator/TM/DCARK/nandipl_02g.bin
	$(Q)cp loader/perma/cipl/new/cipl_01G.bin dist/PC/MagicMemoryCreator/TM/DCARK/cipl_01g.bin
	$(Q)cp loader/perma/cipl/new/cipl_02G.bin dist/PC/MagicMemoryCreator/TM/DCARK/cipl_02g.bin
	$(Q)cp loader/perma/cipl/new/cipl_03G.bin dist/PC/MagicMemoryCreator/TM/DCARK/cipl_03g.bin
	$(Q)cp loader/perma/cipl/new/cipl_04G.bin dist/PC/MagicMemoryCreator/TM/DCARK/cipl_04g.bin
	$(Q)cp loader/perma/cipl/new/cipl_05G.bin dist/PC/MagicMemoryCreator/TM/DCARK/cipl_05g.bin
	$(Q)cp loader/perma/cipl/new/cipl_07G.bin dist/PC/MagicMemoryCreator/TM/DCARK/cipl_07g.bin
	$(Q)cp loader/perma/cipl/new/cipl_09G.bin dist/PC/MagicMemoryCreator/TM/DCARK/cipl_09g.bin
	$(Q)cp loader/perma/cipl/new/cipl_11G.bin dist/PC/MagicMemoryCreator/TM/DCARK/cipl_11g.bin
	$(Q)cp contrib/PSP/IPL/tm_msipl_legacy.bin dist/PC/MagicMemoryCreator/

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

copy-dcark:
	$(Q)cp -r contrib/PC/MagicMemoryCreator dist/PC/
	$(Q)cp dist/SYSCTRL.BIN dist/PC/MagicMemoryCreator/TM/DCARK/kd/ark_systemctrl.prx
	$(Q)cp dist/VSHCTRL.BIN dist/PC/MagicMemoryCreator/TM/DCARK/kd/ark_vshctrl.prx
	$(Q)cp dist/INFERNO.BIN dist/PC/MagicMemoryCreator/TM/DCARK/kd/ark_inferno.prx
	$(Q)cp dist/STARGATE.BIN dist/PC/MagicMemoryCreator/TM/DCARK/kd/ark_stargate.prx
	$(Q)cp dist/POPCORN.BIN dist/PC/MagicMemoryCreator/TM/DCARK/kd/ark_popcorn.prx
	$(Q)cp dist/PSPCOMP.BIN dist/PC/MagicMemoryCreator/TM/DCARK/kd/ark_pspcompat.prx

pack-flash0:
	$(Q)cp core/compat/vita/btcnf/psvbtinf.bin dist/PSVBTINF.BIN
	$(Q)cp core/compat/vita/btcnf/psvbtcnf.bin dist/PSVBTCNF.BIN
	$(Q)cp core/compat/vitapops/btcnf/psvbtxnf.bin dist/PSVBTXNF.BIN
	$(Q)cp core/compat/pentazemin/btcnf/psvbtjnf.bin dist/PSVBTJNF.BIN
	$(Q)cp core/compat/pentazemin/btcnf/psvbtknf.bin dist/PSVBTKNF.BIN
	$(Q)$(PYTHON) contrib/PC/pack/pack.py -p dist/FLASH0.ARK contrib/PC/pack/packlist.txt

cipl:
	$(Q)$(MAKE) PSP_MODEL=01G -C loader/perma/cipl/new/
	$(Q)$(MAKE) PSP_MODEL=02G -C loader/perma/cipl/new/
	$(Q)$(MAKE) PSP_MODEL=03G -C loader/perma/cipl/new/
	$(Q)$(MAKE) PSP_MODEL=04G -C loader/perma/cipl/new/
	$(Q)$(MAKE) PSP_MODEL=05G -C loader/perma/cipl/new/
	$(Q)$(MAKE) PSP_MODEL=07G -C loader/perma/cipl/new/
	$(Q)$(MAKE) PSP_MODEL=09G -C loader/perma/cipl/new/
	$(Q)$(MAKE) PSP_MODEL=11G -C loader/perma/cipl/new/
	$(Q)$(MAKE) -C loader/perma/cipl/installer

msipl:
	$(Q)$(PYTHON) contrib/PC/ipltools/make_ipl.py loader/dc/msipl/newipl/loader/msipl.bin loader/dc/msipl/newipl/loader/ipl.bin reset_block 0x40c0000
	$(Q)bin2c loader/dc/msipl/newipl/loader/ipl.bin loader/dc/msipl/newipl/loader/new_msipl.h new_msipl
	$(Q)bin2c loader/dc/msipl/newipl/loader/msipl.bin loader/dc/msipl/newipl/loader/msipl_raw.h msipl_raw
	$(Q)$(MAKE) PSP_MODEL=01G -C loader/dc/msipl/newipl/payload/
	$(Q)mv loader/dc/msipl/newipl/payload/ipl_01G.bin loader/dc/msipl/newipl/msipl_01g.bin
	$(Q)$(MAKE) PSP_MODEL=02G -C loader/dc/msipl/newipl/payload/
	$(Q)mv loader/dc/msipl/newipl/payload/ipl_02G.bin loader/dc/msipl/newipl/msipl_02g.bin
	$(Q)$(MAKE) PSP_MODEL=03G -C loader/dc/msipl/newipl/payload/
	$(Q)mv loader/dc/msipl/newipl/payload/ipl_03G.bin loader/dc/msipl/newipl/msipl_03g.bin
	$(Q)$(MAKE) PSP_MODEL=04G -C loader/dc/msipl/newipl/payload/
	$(Q)mv loader/dc/msipl/newipl/payload/ipl_04G.bin loader/dc/msipl/newipl/msipl_04g.bin
	$(Q)$(MAKE) PSP_MODEL=05G -C loader/dc/msipl/newipl/payload/
	$(Q)mv loader/dc/msipl/newipl/payload/ipl_05G.bin loader/dc/msipl/newipl/msipl_05g.bin
	$(Q)$(MAKE) PSP_MODEL=07G -C loader/dc/msipl/newipl/payload/
	$(Q)mv loader/dc/msipl/newipl/payload/ipl_07G.bin loader/dc/msipl/newipl/msipl_07g.bin
	$(Q)$(MAKE) PSP_MODEL=09G -C loader/dc/msipl/newipl/payload/
	$(Q)mv loader/dc/msipl/newipl/payload/ipl_09G.bin loader/dc/msipl/newipl/msipl_09g.bin
	$(Q)$(MAKE) PSP_MODEL=11G -C loader/dc/msipl/newipl/payload/
	$(Q)mv loader/dc/msipl/newipl/payload/ipl_11G.bin loader/dc/msipl/newipl/msipl_11g.bin
	$(Q)$(MAKE) -C loader/dc/installer

kxploits:
	$(Q)$(MAKE) $@ K=sceUID -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=sceSdGetLastIndex -C loader/live/kernel/kxploit

# Only clean non-library code
cleanobj:
	$(Q)$(MAKE) clean CLEANOBJ=1

clean:
	$(Q)$(MAKE) $@ -C libs
	$(Q)$(MAKE) $@ -C core/compat/psp/rebootex
	$(Q)$(MAKE) $@ -C core/compat/vita/rebootex
	$(Q)$(MAKE) $@ -C core/compat/vitapops/rebootex
	$(Q)$(MAKE) $@ -C core/compat/pentazemin/rebootex
	$(Q)$(MAKE) $@ -C core/compat/psp/btcnf
	$(Q)$(MAKE) $@ -C core/compat/vita/btcnf
	$(Q)$(MAKE) $@ -C core/compat/vitapops/btcnf
	$(Q)$(MAKE) $@ -C core/compat/pentazemin/btcnf
	$(Q)$(MAKE) $@ -C core/systemctrl
	$(Q)$(MAKE) $@ -C core/vshctrl
	$(Q)$(MAKE) $@ -C core/stargate
	$(Q)$(MAKE) $@ -C core/popcorn
	$(Q)$(MAKE) $@ -C core/inferno
	$(Q)$(MAKE) $@ -C core/compat/psp/rebootex
	$(Q)$(MAKE) $@ -C core/compat/vita/btcnf/
	$(Q)$(MAKE) $@ -C core/compat/vita/rebootex
	$(Q)$(MAKE) $@ -C core/compat/vitapops/btcnf/
	$(Q)$(MAKE) $@ -C core/compat/vitapops/rebootex
	$(Q)$(MAKE) $@ -C core/compat/pentazemin/btcnf/
	$(Q)$(MAKE) $@ -C core/compat/pentazemin/rebootex
	$(Q)$(MAKE) $@ -C core/compat/psp
	$(Q)$(MAKE) $@ -C core/compat/vita
	$(Q)$(MAKE) $@ -C core/compat/vitapops
	$(Q)$(MAKE) $@ -C core/compat/pentazemin
	$(Q)$(MAKE) $@ -C extras/menus/recovery
	$(Q)$(MAKE) $@ -C extras/menus/arkMenu
	$(Q)$(MAKE) $@ -C extras/menus/vshmenu
	$(Q)$(MAKE) $@ -C extras/menus/xMenu
	$(Q)$(MAKE) $@ -C extras/modules/iop
	$(Q)$(MAKE) $@ -C extras/modules/peops
	$(Q)$(MAKE) $@ -C extras/modules/xmbctrl
	$(Q)$(MAKE) $@ -C extras/modules/usbdevice
	$(Q)$(MAKE) $@ -C extras/modules/ipl_update
	$(Q)$(MAKE) $@ -C extras/modules/kbooti_update
	$(Q)$(MAKE) $@ -C extras/modules/kpspident
	$(Q)$(MAKE) $@ -C extras/modules/idsregeneration
	$(Q)$(MAKE) $@ -C loader/live/user/linkless_payload
	$(Q)$(MAKE) $@ -C loader/live/user/signed_eboot
	$(Q)$(MAKE) $@ -C loader/live/user/psxloader
	$(Q)$(MAKE) $@ -C loader/live/kernel/kernel_loader
	$(Q)$(MAKE) $@ -C loader/live/kernel/psxloader
	$(Q)$(MAKE) $@ -C loader/live/kernel/chain_loader
	$(Q)$(MAKE) $@ -C loader/live/kernel/kram_dumper
	$(Q)$(MAKE) $@ -C loader/live/kernel/idstorage_dumper
	$(Q)$(MAKE) $@ -C loader/live/kernel/psp_flash_dumper
	$(Q)$(MAKE) $@ -C loader/live/kernel/vita_flash_dumper
	$(Q)$(MAKE) $@ -C loader/live/kernel/pandorizer
	$(Q)$(MAKE) $@ -C loader/rebootex/ms_payloadex
	$(Q)$(MAKE) $@ -C loader/rebootex/nand_payloadex
	$(Q)$(MAKE) $@ -C loader/perma/cipl/classic/mainbinex
	$(Q)$(MAKE) $@ -C loader/perma/cipl/classic/combine
	$(Q)$(MAKE) $@ -C loader/perma/cipl/new/
	$(Q)$(MAKE) $@ -C loader/perma/cipl/installer
	$(Q)$(MAKE) $@ -C loader/dc/dcman
	$(Q)$(MAKE) $@ -C loader/dc/installer
	$(Q)$(MAKE) $@ -C loader/dc/msipl/newipl/loader
	$(Q)$(MAKE) $@ -C loader/dc/msipl/newipl/payload
	$(Q)$(MAKE) $@ -C loader/dc/msipl/mainbinex
	$(Q)$(MAKE) $@ -C loader/dc/tmctrl/rebootex
	$(Q)$(MAKE) $@ -C loader/dc/tmctrl
	$(Q)$(MAKE) $@ -C loader/dc/vunbricker
	$(Q)$(MAKE) $@ K=sceUID -C loader/live/kernel/kxploit
	$(Q)$(MAKE) $@ K=sceSdGetLastIndex -C loader/live/kernel/kxploit
	$(Q)-rm -rf dist *~ | true
	$(Q)$(MAKE) $@ -C extras/apps/updater/
	$(Q)$(MAKE) $@ -C extras/apps/installer/
	$(Q)$(MAKE) $@ -C extras/apps/uninstaller
	$(Q)$(MAKE) $@ -C loader/perma/cipl/new/ipl_stage1_payload
	$(Q)$(MAKE) $@ -C loader/perma/cipl/new/ipl_stage2_payload
	$(Q)rm -f extras/apps/updater/ARK_01234.PKG | true
	$(Q)rm -f extras/apps/updater/EBOOT_PSP.PBP | true
	$(Q)rm -f extras/apps/updater/EBOOT_GO.PBP | true
	$(Q)$(PYTHON) contrib/PC/scripts/cleandeps.py
	$(Q)find -name 'THEME.ARK' -exec rm {} \;
	$(Q)rm -f extras/menus/arkMenu/LANG.ARK
	$(Q)rm -rf loader/vpk/bin/save/ARK_01234
	$(Q)rm -f loader/vpk/bin/psx/ps1cfw_enabler.suprx
	$(Q)rm -f loader/dc/tmctrl/tmctrl.h
	$(Q)rm -f loader/dc/btcnf/*.bin
	$(Q)rm -f loader/dc/msipl/newipl/*.bin

subdirs: $(SUBDIRS)

$(filter-out libs, $(SUBDIRS)): libs
	$(Q)$(MAKE) $(OPT) -C $@

libs:
	$(Q)$(MAKE) $(OPT) -C $@

arkmenu: libs
	$(Q)$(MAKE) $@ -C extras/menus/arkMenu

vshmenu: libs
	$(Q)$(MAKE) $@ -C extras/menus/vshmenu

xmenu: libs
	$(Q)$(MAKE) $@ -C extras/menus/xMenu

recovery: libs
	$(Q)$(MAKE) $@ -C extras/menus/recovery

mkdir-dist:
	$(Q)mkdir dist | true
	$(Q)mkdir dist/PC | true
	$(Q)mkdir dist/PSP | true
	$(Q)mkdir dist/PSVita | true
	$(Q)mkdir dist/UPDATE | true
	$(Q)mkdir dist/ARK_Loader | true
	$(Q)mkdir dist/PSP/Infinity | true
	$(Q)mkdir dist/PSP/ARK_Uninstaller | true
	$(Q)mkdir dist/PSP/ARK_DC | true
	$(Q)mkdir dist/PSP/ARK_cIPL | true
	$(Q)mkdir dist/PSP/ARK_Full_Installer | true
	$(Q)mkdir dist/PSP/Pops4Tool | true
	$(Q)mkdir dist/PSP/Pops4Tool/TT | true
	$(Q)mkdir dist/PSP/Pops4Tool/DT | true
	$(Q)mkdir dist/PSP/Pops4Tool/kd | true
	$(Q)mkdir dist/PSP/Pops4Tool/vsh | true
	$(Q)mkdir dist/PSP/Pops4Tool/vsh/module | true
	$(Q)mkdir dist/PSVita/Standalone | true
	$(Q)mkdir dist/PSVita/PS1CFW | true
	$(Q)mkdir dist/PSVita/PS1CFW/SCPS10084 | true

-include $(ARKROOT)/.config
include $(ARKROOT)/common/make/quiet.mak
include $(ARKROOT)/common/make/mod_enc.mak
