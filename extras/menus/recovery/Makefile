
TARGET = ark_recovery

XMBCTRL_OBJS = \
	$(ARKROOT)/extras/modules/xmbctrl/config.o \
	$(ARKROOT)/extras/modules/xmbctrl/list.o \
	$(ARKROOT)/extras/modules/xmbctrl/settings.o \
	$(ARKROOT)/extras/modules/xmbctrl/plugins.o

C_OBJS =  main.o proshell.o usb.o submenu.o
OBJS = $(C_OBJS) $(XMBCTRL_OBJS)

CFLAGS = -std=c99 -Os -G0 -Wall
INCDIR = $(ARKROOT)/common/include $(ARKROOT)/extras/modules/xmbctrl $(ARKROOT)/extras/modules/xmbctrl/include
LIBDIR = $(ARKROOT)/libs

LIBS = -lpspsystemctrl_user -lpspkubridge -lpspusb -lpspusbdevice_driver

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS = -mno-crt0 -nostartfiles
BUILD_PRX = 1

all: $(TARGET).prx
	psp-packer $(TARGET).prx

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
