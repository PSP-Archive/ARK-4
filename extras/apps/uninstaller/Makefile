TARGET = uninstaller

OBJS = stubkk.o main.o

CFLAGS = -Os -G0 -Wall -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

INCDIR = $(ARKROOT)/common/include
LIBDIR = $(ARKROOT)/libs


EXTRA_TARGETS = EBOOT.PBP

PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_TITLE = ARK-4 Uninstaller

BUILD_PRX = 1

PSP_FW_VERSION = 660

LIBS = -lpsppower -lpspsystemctrl_user

#include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
#include $(ARKROOT)/common/make/quiet.mak
