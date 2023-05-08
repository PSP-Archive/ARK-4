TARGET = hax
OBJS = crt0.o main.o

INCDIR = 
CFLAGS = -Os -G0 -Wall -fno-pic $(BFLAGS) -fdata-sections -ffunction-sections -Wl,--gc-sections
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti 
ASFLAGS = $(CFLAGS)

LIBS =
LIBDIR =
LDFLAGS = -nostartfiles  -T linkfile.l -fdata-sections -ffunction-sections -Wl,--gc-sections

PSPSDK=$(shell psp-config --pspsdk-path)
include ../common/build.mak
