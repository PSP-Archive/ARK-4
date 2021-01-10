TARGET_LIB=libansic.a

C_OBJS = \
memmove.o \
memcmp.o \
memcpy.o \
memset.o \
strcpy.o \
strlen.o \
strchr.o \
strcmp.o \
memfind.o \
strhex32.o \
strbeginswith.o \
strreplaceall.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

CC=psp-gcc
INCDIR = $(PROVITA)/common/include
CFLAGS = -Wall -Os -G0 -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

include $(PROVITA)/common/make/global.mak
include $(PROVITA)/common/make/beauty_lib.mak
