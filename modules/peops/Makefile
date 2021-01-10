TARGET = peops
SPU_OBJS = spu/decode_xa.o spu/spu.o spu/registers.o
C_OBJS = main.o psp.o audio.o $(SPU_OBJS) imports.o exports.o gamesettings.o
OBJS = $(C_OBJS)

all: $(TARGET).prx

INCDIR = $(PROVITA)/common/include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division

# POPS Version
CFLAGS += -DPOPSMAN_VERSION=$(POPSMAN_VERSION) -DPOPS_VERSION=$(POPS_VERSION)

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

PSP_FW_VERSION = 660

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

#USE_KERNEL_LIBC = 1
#USE_KERNEL_LIBS = 1

LIBDIR = $(PROVITA)/libs
LDFLAGS = -nostartfiles
LIBS = -lpspsystemctrl_user -lpspaudio -lgraphics

include $(PROVITA)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
#include $(PSPSDK)/lib/build.mak
include $(PSPSDK)/lib/build_prx.mak
include $(PROVITA)/common/make/beauty.mak
