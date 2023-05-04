release: all

TARGET = idsregeneration
OBJS = main.o aes_core.o exports.o

INCDIR = $(ARKROOT)/common/include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LDFLAGS = -nostdlib -nodefaultlibs
LIBDIR = $(ARKROOT)/libs
LIBS = -lpspdisplay_driver -lpspnand_driver -lpspsystemctrl_kernel -lpsplflash_fatfmt

all:
	psp-packer $(TARGET).prx

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
