TARGET = iop
OBJS = main.o exports.o

INCDIR = $(ARKROOT)/common/include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1


LIBDIR = $(ARKROOT)/libs
LIBS = -lpspsystemctrl_kernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
