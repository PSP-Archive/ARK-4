ARKROOT ?= ../../

MODEL ?= PSP

PYTHON ?= python3

TARGET = updater
C_OBJS = main.o
OBJS = $(C_OBJS)
all: $(TARGET).prx
CFLAGS = -std=c99 -Os -G0 -Wall
INCDIR = $(ARKROOT)/common/include
LIBDIR = $(ARKROOT)/libs

LIBS = -lpspsystemctrl_user -lansic

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS =
BUILD_PRX = 1

PSP_EBOOT_TITLE += ARK Updater

EXTRA_TARGETS = EBOOT.PBP

PSP_EBOOT_PSAR = ARK_01234.PKG

EBOOT_$(MODEL).PBP: $(EXTRA_TARGETS)
	$(Q)mv $(EXTRA_TARGETS) EBOOT_$(MODEL).PBP

ARK_01234.PKG:
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/pack/pack.py -p ARK_01234.PKG packlist.txt -s

all: ARK_01234.PKG EBOOT_$(MODEL).PBP
	$(Q)rm PARAM.SFO

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak

PARAM.SFO:
	$(Q)cp PARAM_$(MODEL).SFO PARAM.SFO
