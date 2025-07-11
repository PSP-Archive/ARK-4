TARGET=libpng

C_OBJS = \
	main.o \
	exports.o \
	png.o \
	pngerror.o \
	pngget.o \
	pngmem.o \
	pngpread.o \
	pngread.o \
	pngrio.o \
	pngrtran.o \
	pngrutil.o \
	pngset.o \
	pngtrans.o \
	pngwio.o \
	pngwrite.o \
	pngwtran.o \
	pngwutil.o
OBJS = $(C_OBJS)

all:
	psp-packer $(TARGET).prx

PSPSDK=$(shell psp-config --pspsdk-path)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -std=c99 -O2 -G0
CXXFLAGS = $(CFLAGS)	
ASFLAGS = $(CFLAGS)
LIBDIR = $(ARKROOT)/libs
LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lz -lm -lpspsysc_user

include $(PSPSDK)/lib/build.mak
