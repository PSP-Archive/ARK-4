TARGET = vitacompat

C_OBJS =    \
	main.o   \
	vlf.o     \
	syspatch.o  \
	vitaflash.o   \
	popspatch.o    \
	filesystem.o    \
	fatef.o          \
	$(ARKROOT)/libs/libsploit/patches.o \
	$(ARKROOT)/core/compat/psp/cwpatch.o \
	$(ARKROOT)/core/compat/pentazemin/vitamem.o \
	$(ARKROOT)/core/compat/pentazemin/flashfs.o \
	$(ARKROOT)/core/systemctrl/src/dummy.o \
		
OBJS = \
	$(C_OBJS) imports.o

all: $(TARGET).prx
INCDIR = $(ARKROOT)/common $(ARKROOT)
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic

CFLAGS += -I psxspu/ -I $(ARKROOT)/common/include/ -I $(ARKROOT)/core/systemctrl/include/ -I $(ARKROOT)/libs/graphics/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsystemctrl_kernel

PSP_FW_VERSION = 660

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
