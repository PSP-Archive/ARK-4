TARGET = vshctrl150

OBJS = \
	stubkk.o \
	vshmenu.o \
	main.o \
	$(ARKROOT)/libs/ansi-c/strcasecmp.o

all: $(TARGET).prx
INCDIR = $(ARKROOT)/common/include/ $(ARKROOT)/libs/graphics/ $(ARKROOT)
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic $(addprefix -I, $(INCDIR))

CFLAGS += -I include -fshort-wchar

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs lib
LDFLAGS =  -nostartfiles
LIBS = -lpspsystemctrl_kernel -lcolordebugger

PSP_FW_VERSION = 150

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
