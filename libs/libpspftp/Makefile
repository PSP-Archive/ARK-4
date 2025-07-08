TARGET=libpspftp.a

all: $(TARGET)
STUBSRC = libpspftp.S
OBJS = \
	libpspftp_0000.o \
	libpspftp_0001.o \
	libpspftp_0002.o \
	libpspftp_0003.o \
	libpspftp_0004.o \
	libpspftp_0005.o \
	libpspftp_0006.o \
	libpspftp_0007.o \
	libpspftp_0008.o \
	libpspftp_0009.o \
	libpspftp_0010.o \
	libpspftp_0011.o \
	libpspftp_0012.o \
	libpspftp_0013.o \
	libpspftp_0014.o \
	libpspftp_0015.o \
	libpspftp_0016.o \
	libpspftp_0017.o \
	libpspftp_0018.o \


PSPSDK=$(shell psp-config --pspsdk-path)

CC=psp-gcc
INCDIR = 
CFLAGS = -std=c99 -Os -G0 -Wall -fno-builtin-printf -I$(PSPSDK)/include
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS=-nodefaultlibs

LIBDIR =

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_stub.mak
