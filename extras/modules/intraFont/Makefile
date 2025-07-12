TARGET = intraFont
OBJS = \
	main.o \
	exports.o \
	libccc.o   \
	intraFont.o \


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

USE_PSPSDK_LIBS = 1
USE_PSPSDK_LIBC = 1

LIBS = -lm -lpspsysc_user -lguglue

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
