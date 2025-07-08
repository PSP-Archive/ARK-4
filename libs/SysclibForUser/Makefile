TARGET=libpspsysc_user.a

all: $(TARGET)
STUBSRC = SysclibForUser.S
OBJS = \
	SysclibForUser_0000.o \
	SysclibForUser_0001.o \
	SysclibForUser_0002.o \
	SysclibForUser_0003.o \
	SysclibForUser_0004.o \
	SysclibForUser_0005.o \
	SysclibForUser_0006.o \

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
