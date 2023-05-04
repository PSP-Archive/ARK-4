TARGET = pentazemin

C_OBJS =   \
	main.o  \
	funcs.o  \
	vitamem.o \
	syspatch.o \
	io_patch.o  \
	loadexec.o   \
	adrenaline.o  \
	init_patch.o   \
	power_patch.o   \
	LflashFatfmt.o   \
	impose_patch.o    \
	utility_patch.o    \
	$(ARKROOT)/core/compat/vita/vlf.o \
	$(ARKROOT)/common/utils/patches.o   \
	$(ARKROOT)/core/systemctrl/src/dummy.o \
	$(ARKROOT)/core/systemctrl/src/gameinfo.o \
		
OBJS = \
	$(C_OBJS) imports.o

all: $(TARGET).prx
INCDIR = $(ARKROOT)/common $(ARKROOT)
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic -fshort-wchar -mno-check-zero-division

CFLAGS += -I $(ARKROOT)/common/include/ -I $(ARKROOT)/core/systemctrl/include/ -I $(ARKROOT)/libs/graphics/

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
