TARGET=libpspvshctrl.a

all: $(TARGET)
STUBSRC = VshCtrl.S
OBJS = \
	VshCtrl_0000.o \
	VshCtrl_0001.o \
	VshCtrl_0002.o \
	VshCtrl_0003.o \
	VshCtrl_0004.o \
	VshCtrl_0005.o \
	VshCtrl_0006.o \
	VshCtrl_0007.o \
	VshCtrl_0008.o \
	VshCtrl_0009.o \
	VshCtrl_0010.o \
	VshCtrl_0011.o \
	VshCtrl_0012.o \
	VshCtrl_0013.o \

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
