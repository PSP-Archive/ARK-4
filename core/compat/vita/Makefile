TARGET = vitacompat

SPU_OBJS = \
	psxspu/dma.o \
	psxspu/out.o \
	psxspu/registers.o \
	psxspu/spu.o \
	psxspu/xa.o \
	psxspu/nullsnd.o \

C_OBJS =    \
	main.o   \
	vitamem.o \
	iopatch.o  \
	syspatch.o  \
	vitaflash.o  \
	popspatch.o   \
	filesystem.o   \
	$(ARKROOT)/common/functions.o \
	$(ARKROOT)/core/systemctrl/src/dummy.o \
	$(ARKROOT)/core/systemctrl/src/exitgame.o \
	$(ARKROOT)/core/systemctrl/src/gameinfo.o \
		
OBJS = \
	$(C_OBJS) imports.o

all: $(TARGET).prx
INCDIR = $(ARKROOT)/common $(ARKROOT)
CFLAGS = -Os -G0 -Wall -fno-pic -std=gnu11

CFLAGS += -I psxspu/ -I $(ARKROOT)/common/include/ -I $(ARKROOT)/libs/graphics/ -I $(ARKROOT)/core/systemctrl/include/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsemaphore -lgraphics -lcolordebugger -lpspsystemctrl_kernel -lansic

PSP_FW_VERSION = 660

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
