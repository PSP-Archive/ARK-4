TARGET = ark150addoninstaller
OBJS = \
	pspbtcnf_game.h \
	pspbtcnf.h \
	reboot150.h \
	systemctrl150.h \
	tmctrl150.h \
	ark_vshctrl150.h \
	ark_satelite150.h \
	mainbinex.h \
	msipl_raw.h \
	main.o \
	pspDecrypt.o \
	pspPSAR.o \

CFLAGS = -std=c99 -Os -G0 -Wall
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include

LIBS = -lpspkubridge -lpsppower

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS = -L $(ARKROOT)/libs
BUILD_PRX = 1
PSP_EBOOT_ICON = 
PSP_EBOOT_TITLE += ARK 150 Addon Installer

EXTRA_TARGETS = EBOOT.PBP

PSP_FW_VERSION = 660

all: $(TARGET).prx

pspbtcnf_game.h:
	$(Q)bin2c ../btcnf/pspbtcnf_game.txt pspbtcnf_game.h pspbtcnf_game

pspbtcnf.h:
	$(Q)bin2c ../btcnf/pspbtcnf.txt pspbtcnf.h pspbtcnf

ark_vshctrl150.h:
	$(Q)bin2c ../vshctrl150/vshctrl150.prx ark_vshctrl150.h ark_vshctrl150

ark_satelite150.h:
	$(Q)bin2c ../vshmenu150/satelite.prx ark_satelite150.h ark_satelite150

reboot150.h:
	$(Q)bin2c $(ARKROOT)/extras/150kernel/reboot150/reboot150.prx reboot150.h reboot150

systemctrl150.h:
	$(Q)bin2c $(ARKROOT)/extras/150kernel/systemctrl150/systemctrl150.prx systemctrl150.h systemctrl150

tmctrl150.h:
	$(Q)bin2c $(ARKROOT)/extras/150kernel/tmctrl150/tmctrl150.prx tmctrl150.h tmctrl150

mainbinex.h:
	$(Q)bin2c $(ARKROOT)/extras/150kernel/msipl/mainbinex/mainbinex mainbinex.h mainbinex

msipl_raw.h:
	$(Q)bin2c $(ARKROOT)/extras/150kernel/msipl/stage1/msipl msipl_raw.h msipl_raw

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
