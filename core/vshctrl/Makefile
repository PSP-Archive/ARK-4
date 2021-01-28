TARGET = vshctrl
OBJS = main.o vshpatch.o xmbiso.o isoreader.o virtual_pbp.o dirent_track.o disctype.o
IMPORTS = imports.S

INCDIR = $(PROVITA)/common/include $(PROVITA)/modules/systemctrl/include
CFLAGS = -Os -G0 -Wall -fno-pic

CFLAGS += -DCONFIG_661=1
PSP_FW_VERSION = 661

OBJS += $(IMPORTS)
all: $(TARGET).prx
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(PROVITA)/libs/
LDFLAGS =  -nostartfiles
LIBS = -lpspsystemctrl_kernel -lansic -lcolordebugger

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
