TARGET = vitapops

C_OBJS = \
	main.o \
	syspatch.o \
	popsdisplay.o \
	$(ARKROOT)/libs/libsploit/patches.o \
	$(ARKROOT)/core/compat/vita/vitaflash.o \
		
OBJS = \
	$(C_OBJS) imports.o

all: $(TARGET).prx
INCDIR = $(ARKROOT)/common $(ARKROOT)
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic

CFLAGS += -I include -I $(ARKROOT)/common/include/ -I $(ARKROOT)/libs/graphics/ -I $(ARKROOT)/modules/systemctrl/include/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsystemctrl_kernel

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lcolordebugger -lgraphics
endif

PSP_FW_VERSION = 660

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
