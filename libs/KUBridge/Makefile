TARGET=libpspkubridge.a

all: $(TARGET)
STUBSRC = KUBridge.S
OBJS = \
	KUBridge_0000.o \
	KUBridge_0001.o \
	KUBridge_0002.o \
	KUBridge_0003.o \
	KUBridge_0004.o \
	KUBridge_0005.o \
	KUBridge_0006.o \
	KUBridge_0007.o \
	KUBridge_0008.o \
	KUBridge_0009.o \
	KUBridge_0010.o \
	KUBridge_0011.o \
	KUBridge_0012.o \
	KUBridge_0013.o \
	KUBridge_0014.o \
	KUBridge_0015.o \
	KUBridge_0016.o \
	KUBridge_0017.o \
	KUBridge_0018.o \

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
