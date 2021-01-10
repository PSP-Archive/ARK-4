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

INCDIR = $(PROVITA)/libs/libpspvram $(PROVITA)/libs/libpng $(PROVITA)/libs/libjpeg $(PROVITA)/libs/libpspmath $(PROVITA)/common/include $(PSPSDK)/include
CFLAGS = -G0 -Wall -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =
LDFLAGS = -lstdc++

include $(PROVITA)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty_lib.mak
