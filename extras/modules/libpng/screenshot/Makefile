TARGET = screenshot
OBJS = main.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall -fno-strict-aliasing
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS = -lpng -lz -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Screenshot

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
