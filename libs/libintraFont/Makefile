TARGET=libintraFont_prx.a

all: $(TARGET)
STUBSRC = libintraFont.S
OBJS = \
	intrafont_0000.o \
	intrafont_0001.o \
	intrafont_0002.o \
	intrafont_0003.o \
	intrafont_0004.o \
	intrafont_0005.o \
	intrafont_0006.o \
	intrafont_0007.o \
	intrafont_0008.o \
	intrafont_0009.o \
	intrafont_0010.o \
	intrafont_0011.o \
	intrafont_0012.o \
	intrafont_0013.o \
	intrafont_0014.o \
	intrafont_0015.o \
	intrafont_0016.o \
	intrafont_0017.o \
	intrafont_0018.o \
	intrafont_0019.o \
	intrafont_0020.o \
	intrafont_0021.o \
	intrafont_0022.o \

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
