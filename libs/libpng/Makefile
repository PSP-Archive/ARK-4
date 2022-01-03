TARGET_LIB=libpng.a

C_OBJS = \
png.o \
pngerror.o \
pngget.o \
pngmem.o \
pngpread.o \
pngread.o \
pngrio.o \
pngrtran.o \
pngrutil.o \
pngset.o \
pngtrans.o \
pngwio.o \
pngwrite.o \
pngwtran.o \
pngwutil.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -std=c99 -O2 -G0
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LIBDIR =
LDFLAGS = -lstdc++

include $(ARKROOT)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty_lib.mak
