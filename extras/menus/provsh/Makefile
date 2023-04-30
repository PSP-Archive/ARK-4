TARGET = proshell
C_OBJS = main.o
OBJS = $(C_OBJS)
all: $(TARGET).prx
CFLAGS = -std=c99 -Os -G0 -Wall
INCDIR = $(ARKROOT)/common/include
LIBDIR = $(ARKROOT)/libs

LIBS = -lpspsystemctrl_user -lansic

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS =
BUILD_PRX = 1

PSP_EBOOT_TITLE += PRO Shell

EXTRA_TARGETS = EBOOT.PBP

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
