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

INCDIR = $(PROVITA)/common/include $(PSPSDK)/include
CFLAGS = -O2 -G0
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LIBDIR =
LDFLAGS = -lstdc++

include $(PROVITA)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty_lib.mak
