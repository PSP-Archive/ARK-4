TARGET = vitacompat

SPU_OBJS = \
	psxspu/dma.o \
	psxspu/out.o \
	psxspu/registers.o \
	psxspu/spu.o \
	psxspu/xa.o \
	psxspu/nullsnd.o \

C_OBJS =   \
	main.o  \
	vitamem.o \
	syspatch.o \
	vitaflash.o \
	popspatch.o  \
	filesystem.o  \
	$(PROVITA)/common/functions.o \
	$(PROVITA)/core/systemctrl/src/dummy.o \
	$(PROVITA)/core/systemctrl/src/exitgame.o \
	$(PROVITA)/core/systemctrl/src/gameinfo.o \
		
OBJS = \
	$(C_OBJS) imports.o

all: $(TARGET).prx
INCDIR = $(PROVITA)/common $(PROVITA)
CFLAGS = -Os -G0 -Wall -fno-pic -std=gnu11

CFLAGS += -I psxspu/ -I $(PROVITA)/common/include/ -I $(PROVITA)/libs/graphics/ -I $(PROVITA)/core/systemctrl/include/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(PROVITA)/libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsemaphore -lcolordebugger -lgraphics -lpspsystemctrl_kernel

PSP_FW_VERSION = 660

include $(PROVITA)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty.mak
