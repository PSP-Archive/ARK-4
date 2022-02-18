TARGET = peops
OBJS = main.o psp.o audio.o $(SPU_OBJS) imports.o exports.o
SPU_OBJS = spu/decode_xa.o spu/spu.o spu/registers.o

INCDIR = ../../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

#USE_KERNEL_LIBC = 1
#USE_KERNEL_LIBS = 1

LIBDIR = ../../lib
LIBS = -lpspsystemctrl_user -lpspaudio

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak