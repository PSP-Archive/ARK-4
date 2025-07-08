TARGET = pspftp
OBJS = \
	main.o \
	exports.o \
	src/ftp.o \
	src/ftpd.o \
	src/sutils.o \
	src/loadutil.o \
	src/psp_cfg.o \
	src/psp_init.o \
	src/ftpclient.o \


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

LIBS = -lpspnet -lpspnet_inet -lpspnet_apctl -lpspsystemctrl_user -lpspsysc_user -lansic

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
