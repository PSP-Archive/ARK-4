TARGET = iplsdk
OBJS = fat.o kirk.o ms.o printf.o syscon.o sysreg.o uart.o

INCDIR = 
CFLAGS = -Os -G0 -Wall -std=c99 -fno-pic $(BFLAGS) -fdata-sections -ffunction-sections -Wl,--gc-sections
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti 
ASFLAGS = $(CFLAGS)

LIBS =
LIBDIR =
LDFLAGS = -fdata-sections -ffunction-sections -Wl,--gc-sections

PSPSDK=$(shell psp-config --pspsdk-path)
include build_lib.mak
