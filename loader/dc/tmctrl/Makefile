TARGET = tmctrl

OBJS = \
	main.o \
	flashemu.o \
	
all: $(TARGET).prx tmctrl.h

tmctrl.h: $(TARGET).prx
	$(Q)bin2c $(TARGET).prx tmctrl.h tmctrl

INCDIR = $(ARKROOT)/common $(ARKROOT)
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic -fshort-wchar

CFLAGS += -I include -I $(ARKROOT)/common/include/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs
LDFLAGS = -nostartfiles
LIBS = -lpspsystemctrl_kernel -lpspsemaphore

PSP_FW_VERSION = 660

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
