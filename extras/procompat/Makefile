TARGET = procompat

C_OBJS = \
	main.o \
	$(ARKROOT)/core/pspcompat/syspatch.o \
	$(ARKROOT)/core/pspcompat/high_mem.o  \
	$(ARKROOT)/core/pspcompat/cpuclock.o   \
	$(ARKROOT)/core/pspcompat/usb_charge.o  \
	$(ARKROOT)/common/functions.o \
	$(ARKROOT)/core/systemctrl/src/exitgame.o \
	$(ARKROOT)/core/systemctrl/src/gameinfo.o \
		
OBJS = \
	$(C_OBJS) imports.o

all: $(TARGET).prx
INCDIR = $(ARKROOT)/common $(ARKROOT)
CFLAGS = -Os -G0 -Wall -fno-pic

CFLAGS += -I include -I $(ARKROOT)/common/include/ -I $(ARKROOT)/libs/graphics/ -I $(ARKROOT)/core/systemctrl/include/ -I $(ARKROOT)/core/pspcompat/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsemaphore -lgraphics -lcolordebugger -lpspsystemctrl_kernel

PSP_FW_VERSION = 660

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
