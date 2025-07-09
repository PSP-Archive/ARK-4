TARGET = pspav
OBJS = \
	main.o \
	exports.o \
	imports.o \
	src/at3.o \
	src/pspav.o \
	src/pspav_reader.o \
	src/pspav_decoder.o \
	src/pspav_audio.o \
	src/pspav_video.o \


CFLAGS = -std=c99 -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS = -mno-crt0 -nostartfiles

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = include/
LIBDIR = $(ARKROOT)/libs

all:
	psp-packer $(TARGET).prx

USE_KERNEL_LIBS = 0
USE_KERNEL_LIBC = 1
USE_PSPSDK_LIBS = 0
USE_PSPSDK_LIBC = 0

LIBS = -lpsppower -lpspaudio -lpspatrac3 -lpspmpeg -lpspsystemctrl_user -lpspsysc_user

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
