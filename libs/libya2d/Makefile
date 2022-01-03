TARGET_LIB = libya2d.a

C_OBJS = \
ya2d_main.o \
ya2d_image.o \
ya2d_texture.o \
ya2d_draw.o \
ya2d_utils.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(ARKROOT)/libs/libpspvram $(ARKROOT)/libs/libpng $(ARKROOT)/libs/libjpeg $(ARKROOT)/libs/libpspmath $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -std=c99 -G0 -Wall -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =
LDFLAGS = -lstdc++

include $(ARKROOT)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty_lib.mak
