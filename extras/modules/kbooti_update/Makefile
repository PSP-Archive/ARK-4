TARGET = kbooti_update
OBJS = main.o

all: $(TARGET).prx

CFLAGS = -Os -G0 -Wall -fno-pic -std=c99
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PSP_FW_VERSION = 660

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

LIBDIR = $(ARKROOT)/libs
LDFLAGS = -nostartfiles
LIBS = -lpspsystemctrl_kernel

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
