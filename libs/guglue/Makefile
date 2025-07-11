
TARGET=libguglue.a

all: $(TARGET)
STUBSRC = libguglue.S
OBJS = \
	libguglue_0000.o \
	libguglue_0001.o \
	libguglue_0002.o \
	libguglue_0003.o \
	libguglue_0004.o \
	libguglue_0005.o \
	libguglue_0006.o \
	libguglue_0007.o \
	libguglue_0008.o \
	libguglue_0009.o \
	libguglue_0010.o \
	libguglue_0011.o \
	libguglue_0012.o \
	libguglue_0013.o \
	libguglue_0014.o \

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
