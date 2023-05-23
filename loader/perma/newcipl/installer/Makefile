TARGET = flasher
OBJS = main.o pspIplUpdate.o kpspident.o

LIBS = -lpspkubridge -lpspsystemctrl_user
INCDIR = $(ARKROOT)/common/include
LIBDIR = $(ARKROOT)/libs

CFLAGS = -O2 -G0 -Wall -fshort-wchar
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = 661 ARK cIPL Flasher
PSP_EBOOT_ICON = ICON0.PNG
PSP_FW_VERSION = 660
BUILD_PRX = 1

all: $(TARGET).prx

distclean:
	make clean

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
