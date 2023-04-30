TARGET = vshctrl
OBJS = main.o vshpatch.o xmbiso.o isoreader.o virtual_pbp.o dirent_track.o disctype.o vshmenu.o usbdevice.o custom_update.o hibernation_delete.o $(ARKROOT)/libs/ansi-c/strsafe.o
IMPORTS = imports.o

INCDIR = $(ARKROOT)/common/include $(ARKROOT)/core/systemctrl/include
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic

PSP_FW_VERSION = 660

OBJS += $(IMPORTS)
all: $(TARGET).prx
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs/
LDFLAGS =  -nostartfiles
LIBS = -lpspsystemctrl_kernel -lpspusb -lpspusbdevice_driver -lcolordebugger

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
