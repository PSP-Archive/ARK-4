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

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = include/
LIBDIR = $(ARKROOT)/libs

all:
	psp-packer $(TARGET).prx

USE_PSPSDK_LIBS = 1
USE_PSPSDK_LIBC = 1

LIBS = -lpspnet -lpspnet_inet -lpspnet_apctl

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
