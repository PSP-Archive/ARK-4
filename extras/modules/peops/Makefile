TARGET = peops
OBJS = main.o psp.o audio.o $(SPU_OBJS) exports.o
SPU_OBJS = spu/decode_xa.o spu/spu.o spu/registers.o

INCDIR = $(ARKROOT)/common/include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_PSPSDK_LIBS = 1
USE_PSPSDK_LIBC = 1

LIBDIR = $(ARKROOT)/libs
LIBS = -lpspsystemctrl_user -lpspaudio -lpspsysc_user

all:
	psp-packer $(TARGET).prx

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak