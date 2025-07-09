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
	SysclibForUser_0007.o \
	SysclibForUser_0008.o \
	SysclibForUser_0009.o \
	SysclibForUser_0010.o \
	SysclibForUser_0011.o \
	SysclibForUser_0012.o \
	SysclibForUser_0013.o \
	SysclibForUser_0014.o \
	SysclibForUser_0015.o \
	SysclibForUser_0016.o \
	SysclibForUser_0017.o \
	SysclibForUser_0018.o \
	SysclibForUser_0019.o \
	SysclibForUser_0020.o \
	SysclibForUser_0021.o \
	SysclibForUser_0022.o \
	SysclibForUser_0023.o \
	SysclibForUser_0024.o \
	SysclibForUser_0025.o \
	SysclibForUser_0026.o \
	SysclibForUser_0027.o \
	SysclibForUser_0028.o \
	SysclibForUser_0029.o \
	SysclibForUser_0030.o \
	SysclibForUser_0031.o \
	SysclibForUser_0032.o \
	SysclibForUser_0033.o \
	SysclibForUser_0034.o \
	SysclibForUser_0035.o \
	SysclibForUser_0036.o \

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
