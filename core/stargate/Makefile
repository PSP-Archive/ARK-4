TARGET = stargate
C_OBJS = main.o loadmodule_patch.o nodrm_patch.o io_patch.o key_decrypt.o pspcipher.o hide.o chn_iso.o imports.o
OBJS = $(C_OBJS)
all: $(TARGET).prx
INCDIR = $(ARKROOT)/common/include
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

PSP_FW_VERSION = 660

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsystemctrl_kernel -lpspsemaphore

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
