TARGET_LIB=libpsid.a
C_OBJS = \
rc4.o \
psid.o

ifneq ($(RELEASE), 1)
	C_OBJS += psidcheck.o
endif

all: $(TARGET_LIB)

OBJS = $(C_OBJS)
PSPSDK=$(shell psp-config --pspsdk-path)

CC=psp-gcc
INCDIR = $(ARKROOT)/common/include $(ARKROOT)/common $(PSPSDK)/include
CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_lib.mak
