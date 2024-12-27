TARGET = arkdcinstaller
OBJS = \
	crt0.o \
	tm_msipl.h \
	tm_mloader.h \
	pspbtcnf_dc.h \
	pspbtcnf_02g_dc.h \
	pspbtcnf_03g_dc.h \
	pspbtcnf_04g_dc.h \
	pspbtcnf_05g_dc.h \
	pspbtcnf_07g_dc.h \
	pspbtcnf_09g_dc.h \
	pspbtcnf_11g_dc.h \
	ipl_update.h \
	dcman.h \
	iop.h \
	lflash_fdisk.h \
	pspdecryptmod.h \
	intrafont.h \
	resurrection.h \
	vlf.h \
	idsregen.h \
	main.o \
	pspDecrypt.o \
	pspPSAR.o \


INCDIR = $(ARKROOT) \
	$(ARKROOT)/common/include \
	$(ARKROOT)/libs/libvlf \
	$(ARKROOT)/loader/dc/msipl/newipl/stage2 \
	$(ARKROOT)/loader/dc/msipl/newipl/stage3 \
	$(ARKROOT)/loader/perma/cipl/new/

CFLAGS = -std=c99 -O2 -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

LIBDIR = $(ARKROOT)/libs
LDFLAGS = -nostartfiles -nostdlib -nodefaultlibs
LIBS = -lpspsystemctrl_user -lpspkubridge -lpsppower -lvlfgui -lvlfgu -lvlfutils -lvlflibc 

PSP_FW_VERSION = 660

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = ARK DC Installer
PSP_EBOOT_ICON = despertar.png
PSP_EBOOT_PIC0 = PIC0.PNG

BUILD_PRX = 1
PSP_LARGE_MEMORY = 1

all: $(TARGET).prx

tm_msipl.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/IPL/tm_msipl_legacy.bin tm_msipl.h tm_msipl

tm_mloader.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/IPL/tm_mloader.bin tm_mloader.h tm_mloader

pspbtcnf_dc.h:
	$(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build ../btcnf/pspbtcnf_dc.txt
	$(Q)bin2c ../btcnf/pspbtcnf_dc.bin pspbtcnf_dc.h pspbtcnf_dc

pspbtcnf_02g_dc.h:
	$(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build ../btcnf/pspbtcnf_02g_dc.txt
	$(Q)bin2c ../btcnf/pspbtcnf_02g_dc.bin pspbtcnf_02g_dc.h pspbtcnf_02g_dc

pspbtcnf_03g_dc.h:
	$(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build ../btcnf/pspbtcnf_03g_dc.txt
	$(Q)bin2c ../btcnf/pspbtcnf_03g_dc.bin pspbtcnf_03g_dc.h pspbtcnf_03g_dc

pspbtcnf_04g_dc.h:
	$(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build ../btcnf/pspbtcnf_04g_dc.txt
	$(Q)bin2c ../btcnf/pspbtcnf_04g_dc.bin pspbtcnf_04g_dc.h pspbtcnf_04g_dc

pspbtcnf_05g_dc.h:
	$(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build ../btcnf/pspbtcnf_05g_dc.txt
	$(Q)bin2c ../btcnf/pspbtcnf_05g_dc.bin pspbtcnf_05g_dc.h pspbtcnf_05g_dc

pspbtcnf_07g_dc.h:
	$(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build ../btcnf/pspbtcnf_07g_dc.txt
	$(Q)bin2c ../btcnf/pspbtcnf_07g_dc.bin pspbtcnf_07g_dc.h pspbtcnf_07g_dc

pspbtcnf_09g_dc.h:
	$(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build ../btcnf/pspbtcnf_09g_dc.txt
	$(Q)bin2c ../btcnf/pspbtcnf_09g_dc.bin pspbtcnf_09g_dc.h pspbtcnf_09g_dc

pspbtcnf_11g_dc.h:
	$(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build ../btcnf/pspbtcnf_11g_dc.txt
	$(Q)bin2c ../btcnf/pspbtcnf_11g_dc.bin pspbtcnf_11g_dc.h pspbtcnf_11g_dc

dcman.h:
	$(Q)bin2c ../dcman/dcman.prx dcman.h dcman

resurrection.h:
	$(Q)bin2c ../vunbricker/resurrection.prx resurrection.h resurrection

ipl_update.h:
	$(Q)bin2c $(ARKROOT)/extras/modules/ipl_update/ipl_update.prx ipl_update.h ipl_update

iop.h:
	$(Q)bin2c $(ARKROOT)/extras/modules/iop/iop.prx iop.h iop

lflash_fdisk.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/GAME/ARK_DC/lflash_fdisk.prx lflash_fdisk.h lflash_fdisk

pspdecryptmod.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/GAME/ARK_DC/pspdecrypt.prx pspdecryptmod.h pspdecrypt

intrafont.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/GAME/ARK_DC/intraFont.prx intrafont.h intrafont

vlf.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/GAME/ARK_DC/vlf.prx vlf.h vlf

idsregen.h:
	$(Q)bin2c $(ARKROOT)/extras/modules/idsregeneration/idsregeneration.prx idsregen.h idsregen

distclean:
	make clean

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include ./build.mak
include $(ARKROOT)/common/make/beauty.mak
