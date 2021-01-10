TARGET = proshell
C_OBJS = main.o
OBJS = $(C_OBJS)
all: $(TARGET).prx
CFLAGS = -Os -G0 -Wall
INCDIR = $(PROVITA)/common/include
LIBDIR = $(PROVITA)/libs

LIBS = -lpspsystemctrl_user

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS =
BUILD_PRX = 1

PSP_EBOOT_TITLE += PRO Shell

EXTRA_TARGETS = EBOOT.PBP

include $(PROVITA)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty.mak
