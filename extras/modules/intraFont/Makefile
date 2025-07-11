TARGET = libintrafont

C_OBJS = \
	main.o \
	exports.o \
	libccc.o   \
	intraFont.o \

OBJS = $(C_OBJS)
LIBS = -lm -lpspsysc_user -lguglue

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

all:
	psp-packer $(TARGET).prx


PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -std=c99 -G0 -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS = -mno-crt0 -nostartfiles
LIBDIR = $(ARKROOT)/libs

include $(PSPSDK)/lib/build.mak
