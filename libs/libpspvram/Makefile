TARGET_LIB = libpspvram.a

C_OBJS = vram.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -std=c99 -G0 -mno-explicit-relocs -O2 -Wall -Werror -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

include $(ARKROOT)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty_lib.mak

