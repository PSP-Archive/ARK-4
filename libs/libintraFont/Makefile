TARGET_LIB = libintraFont.a

C_OBJS = \
intraFont.o \
libccc.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -std=c99 -G0 -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

include $(ARKROOT)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty_lib.mak
