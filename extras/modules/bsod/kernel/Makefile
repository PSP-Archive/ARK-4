TARGET = bsod
OBJS = main.o \
	exports.o \
	exception.o \

INCDIR =  -I $(ARKROOT)/common/include $(ARKROOT)/core/systemctrl/include $(ARKROOT)/libs/graphics
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -std=c99 -DDEBUG=1
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION=660

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = $(ARKROOT)/libs
LIBS = -lpspsystemctrl_kernel -lgraphics -lcolordebugger

all:
	psp-packer $(TARGET).prx

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

clean:
	$(Q)rm -f *.bin *.elf *.prx *.o $(ARKROOT)/core/systemctrl/debug/*.o