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
	vlf.o    \
	syspatch.o  \
	gameinfo.o   \
	vitaflash.o   \
	popspatch.o    \
	filesystem.o    \
	$(ARKROOT)/common/utils/patches.o \
	$(ARKROOT)/core/systemctrl/src/dummy.o \
	$(ARKROOT)/core/systemctrl/src/gameinfo.o \
		
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
LIBS = -lpspsemaphore -lpspsystemctrl_kernel

PSP_FW_VERSION = 660

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
