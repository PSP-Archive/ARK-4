# Number of Compilation Threads
OPT=-j8

PYTHON = $(shell which python2)
PROVITA ?= $(CURDIR)
SAVE ?= -1
K ?= sceNetMPulldown
FLASH_DUMP ?= 0

export DEBUG PROVITA K FLASH_DUMP
export POPSMAN_VERSION POPS_VERSION

SUBDIRS = libs contrib/PC/prxencrypter modules/systemctrl modules/ISODrivers/galaxy modules/stargate modules/exitgame menu/provsh menu/arkMenu menu/xMenu modules/popcorn modules/peops modules/ISODrivers/inferno modules/rebootbuffer loader/stage2/live loader/kxploit loader/stage1/linkless_payload loader/stage1/live_eboot contrib/PC/btcnf
.PHONY: subdirs $(SUBDIRS) cleanobj clean cleanobj distclean copy-bin mkdir-dist encrypt-prx

all: subdirs mkdir-dist encrypt-prx copy-bin

copy-bin: loader/stage1/linkless_payload/h.bin loader/stage1/live_eboot/EBOOT.PBP loader/kxploit/k.bin contrib/PC/btcnf/psvbtinf.bin contrib/PC/btcnf/psvbtxnf.bin contrib/PC/btcnf/psvbtnnf.bin contrib/PSP/fake.cso menu/provsh/EBOOT.PBP
#	Common installation
#	$(Q)cp -r contrib/PSP/SAVEDATA/ARK_01234 dist/PSP/SAVEDATA/
#	$(Q)cp loader/stage2/live/ark.bin dist/PSP/SAVEDATA/ARK_01234/ARK.BIN
#	$(Q)cp loader/kxploit/k.bin dist/PSP/SAVEDATA/ARK_01234/K.BIN
#	$(Q)cp dist/FLASH0.ARK dist/PSP/SAVEDATA/ARK_01234/FLASH0.ARK
#	$(Q)cp dist/NPDRM.PRX dist/PSP/SAVEDATA/ARK_01234/NPDRM.PRX
#	$(Q)cp menu/provsh/EBOOT.PBP dist/PSP/SAVEDATA/ARK_01234/MBOOT.PBP
	$(Q)cp loader/stage1/live_eboot/EBOOT.PBP dist/PSP/GAME/ARK_Live/EBOOT.PBP
	$(Q)cp menu/provsh/EBOOT.PBP dist/PSP/GAME/ARK_Live/MBOOT.PBP
	$(Q)cp dist/NPDRM.PRX dist/PSP/GAME/ARK_Live/NPDRM.PRX
	$(Q)cp dist/FLASH0.ARK dist/PSP/GAME/ARK_Live/FLASH0.ARK
	$(Q)cp loader/kxploit/k.bin dist/PSP/GAME/ARK_Live/K.BIN
	$(Q)cp loader/stage2/live/ark.bin dist/PSP/GAME/ARK_Live/ARK.BIN
	$(Q)cp loader/stage1/live_eboot/eboot/EBOOT.PBP dist/VHBL/EBOOT.PBP
	$(Q)cp menu/provsh/EBOOT.PBP dist/VHBL/MBOOT.PBP
	$(Q)cp dist/NPDRM.PRX dist/VHBL/NPDRM.PRX
	$(Q)cp dist/FLASH0.ARK dist/VHBL/FLASH0.ARK
	$(Q)cp loader/kxploit/k.bin dist/VHBL/K.BIN
	$(Q)cp loader/stage2/live/ark.bin dist/VHBL/ARK.BIN
#	$(Q)cp menu/arkMenu/EBOOT.PBP dist/ARK_01234/VBOOT.PBP
#	$(Q)cp menu/arkMenu/DATA.PKG dist/ARK_01234/DATA.PKG
#	$(Q)cp menu/xMenu/EBOOT.PBP dist/ARK_01234/XBOOT.PBP
#	game exploits
#	$(Q)cp loader/stage1/linkless_payload/h.bin dist/psv/exploit/H.BIN
#	eCFW (bubbles)
#	$(Q)cp loader/stage1/live_eboot/eboot/EBOOT.PBP dist/psv/bubble/PBOOT.PBP
#	live CFW
#	full CFW (infinity)
#	$(Q)cp loader/stage2/infinity/EBOOT.PBP dist/psp/ARK_Infinity/EBOOT.PBP
#	full CFW (CIPL)
#	$(Q)cp loader/stage2/cipl/EBOOT.PBP dist/psp/ARK_CIPL/EBOOT.PBP
	
encrypt-prx: \
	dist/SYSCTRL0.BIN dist/GALAXY00.BIN dist/INFERNO0.BIN dist/STARGATE.BIN dist/EXITGAME.BIN dist/POPCORN0.BIN dist/PEOPS.PRX dist/MARCH330.BIN\
	dist/POPSMAN0.BIN dist/POPS.PRX dist/PSPVMC00.BIN dist/MEDIASYN.BIN dist/MODULEMR.BIN dist/NPDRM.PRX dist/NP966000.BIN
	$(Q)cp contrib/PC/btcnf/psvbtinf.bin dist/PSVBTINF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtxnf.bin dist/PSVBTXNF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtnnf.bin dist/PSVBTNNF.BIN
	$(Q)cp contrib/PC/btcnf/psvbtmnf.bin dist/PSVBTMNF.BIN
	$(Q)cp contrib/PSP/fake.cso dist/FAKECSO0.BIN
	$(Q)cp contrib/PSP/qsplink.prx dist/QSPLINK.PRX
	$(Q)$(PYTHON) contrib/PC/pack/pack.py -p dist/FLASH0.ARK contrib/PC/pack/packlist.txt
#	in the end always destroy tmp release key cache
	$(Q)-rm -f $(tmpReleaseKey)


# Only clean non-library code
cleanobj:
	$(Q)$(MAKE) clean CLEANOBJ=1

distclean clean:
ifndef CLEANOBJ
	$(Q)$(MAKE) $@ -C libs
endif
	$(Q)$(MAKE) $@ -C modules/rebootbuffer
	$(Q)$(MAKE) $@ -C loader/stage1/live_eboot
	$(Q)$(MAKE) $@ -C loader/stage2/live
	$(Q)$(MAKE) $@ -C loader/kxploit
	$(Q)$(MAKE) $@ -C loader/stage1/linkless_payload
	$(Q)$(MAKE) $@ -C modules/systemctrl
	$(Q)$(MAKE) $@ -C modules/ISODrivers/galaxy
	$(Q)$(MAKE) $@ -C modules/stargate
	$(Q)$(MAKE) $@ -C modules/exitgame
	$(Q)$(MAKE) $@ -C menu/provsh
	$(Q)$(MAKE) $@ -C menu/arkMenu
	$(Q)$(MAKE) $@ -C menu/xMenu
	$(Q)$(MAKE) $@ -C modules/popcorn
	$(Q)$(MAKE) $@ -C modules/peops
	$(Q)$(MAKE) $@ -C modules/ISODrivers/inferno
	$(Q)-rm -rf dist *~ | true
	$(Q)-rm -f contrib/PC/btcnf/pspbtinf.bin
	$(Q)-rm -f contrib/PC/btcnf/pspbtmnf.bin
	$(Q)-rm -f contrib/PC/btcnf/pspbtnnf.bin
	$(Q)-rm -f contrib/PC/btcnf/pspbtxnf.bin
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

ark: rebootbuffer

loader: ark

mkdir-dist:
	$(Q)mkdir dist | true
	$(Q)mkdir dist/PSP | true
	$(Q)mkdir dist/VHBL | true
#	$(Q)mkdir dist/Vita_Bubble | true
#	$(Q)mkdir dist/PSP/SAVEDATA/ | true
	$(Q)mkdir dist/PSP/GAME/ | true
	$(Q)mkdir dist/PSP/GAME/ARK_Live | true
#	$(Q)mkdir dist/PSP/ARK_Infinity | true
#	$(Q)mkdir dist/PSP/ARK_CIPL | true
#	$(Q)mkdir dist/PSV/exploit | true

-include $(PROVITA)/.config
include $(PROVITA)/common/make/check.mak
include $(PROVITA)/common/make/quiet.mak
include $(PROVITA)/common/make/mod_enc.mak
