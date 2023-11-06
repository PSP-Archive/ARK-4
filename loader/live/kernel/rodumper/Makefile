TARGET = kram-dumper
OBJS = main.o imports.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSP Kernel RAM Dumper

LIBDIR = 
LIBS = -lpsprtc

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak