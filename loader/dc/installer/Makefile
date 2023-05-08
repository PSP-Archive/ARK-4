TARGET = arkdcinstaller
OBJS = \
	crt0.o \
	tm_msipl.h \
	tm_mloader.h \
	pspbtcnf_dc.h \
	pspbtcnf_02g_dc.h \
	ipl_update.h \
	dcman.h \
	iop.h \
	pspdecryptmod.h \
	intrafont.h \
	resurrection.h \
	vlf.h \
	main.o \
	pspDecrypt.o \
	pspPSAR.o \


INCDIR = $(ARKROOT)/common/include $(ARKROOT)/libs/libvlf $(ARKROOT)
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

BUILD_PRX = 1
PSP_LARGE_MEMORY = 1

all: $(TARGET).prx

tm_msipl.h:
	$(Q)bin2c $(ARKROOT)/contrib/PC/timemachine/tm_msipl_legacy.bin tm_msipl.h tm_msipl

tm_mloader.h:
	$(Q)bin2c $(ARKROOT)/contrib/PC/timemachine/tm_mloader.bin tm_mloader.h tm_mloader

pspbtcnf_dc.h:
	$(Q)bin2c ../btcnf/pspbtcnf_dc.bin pspbtcnf_dc.h pspbtcnf_dc

pspbtcnf_02g_dc.h:
	$(Q)bin2c ../btcnf/pspbtcnf_02g_dc.bin pspbtcnf_02g_dc.h pspbtcnf_02g_dc

dcman.h:
	$(Q)bin2c ../dcman/dcman.prx dcman.h dcman

resurrection.h:
	$(Q)bin2c ../vunbricker/resurrection.prx resurrection.h resurrection

ipl_update.h:
	$(Q)bin2c $(ARKROOT)/extras/modules/ipl_update/ipl_update.prx ipl_update.h ipl_update

iop.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/GAME/ARK_DC/iop.prx iop.h iop

pspdecryptmod.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/GAME/ARK_DC/pspdecrypt.prx pspdecryptmod.h pspdecrypt

intrafont.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/GAME/ARK_DC/intraFont.prx intrafont.h intrafont

vlf.h:
	$(Q)bin2c $(ARKROOT)/contrib/PSP/GAME/ARK_DC/vlf.prx vlf.h vlf

distclean:
	make clean

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include ./build.mak
include $(ARKROOT)/common/make/beauty.mak
