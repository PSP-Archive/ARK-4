TARGET_LIB = libpspvram.a

C_OBJS = vram.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(PROVITA)/common/include $(PSPSDK)/include
CFLAGS = -G0 -mno-explicit-relocs -O2 -Wall -Werror -g
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

include $(PROVITA)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty_lib.mak

