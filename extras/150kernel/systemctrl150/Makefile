TARGET = systemctrl150

OBJS = \
	main.o \
	src/flushcache.o \
	src/hooknids.o \
	src/loadexec.o \
	src/modulemanager.o \
	src/rebootex.o \
	src/sctrl_hen.o \
	src/syspatch.o \

all: $(TARGET).prx
INCDIR = $(ARKROOT)/common/include/ $(ARKROOT)/libs/graphics/ $(ARKROOT)
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic $(addprefix -I, $(INCDIR))

CFLAGS += -I include 

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs lib
LDFLAGS =  -nostartfiles
LIBS = -lpspsemaphore -lpspdisplay_driver_weak -lpspreg_driver_weak

PSP_FW_VERSION = 150

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
