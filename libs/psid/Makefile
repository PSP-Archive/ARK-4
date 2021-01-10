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
INCDIR = $(PROVITA)/common/include $(PROVITA)/common $(PSPSDK)/include
CFLAGS = -Wall -Os -G0 -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

include $(PROVITA)/common/make/global.mak
include $(PROVITA)/common/make/beauty_lib.mak
