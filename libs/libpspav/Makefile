
TARGET=libpspav.a

all: $(TARGET)
STUBSRC = libpspav.S
OBJS = \
	libpspav_0000.o \
	libpspav_0001.o \
	libpspav_0002.o \
	libpspav_0003.o \
	libpspav_0004.o \
	libpspav_0005.o \

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
