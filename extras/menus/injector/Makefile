TARGET = injector
OBJS = main.o
CFLAGS = -std=c99 -Os -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif
LIBDIR = $(ARKROOT)/libs
LIBS = -lpspsystemctrl_kernel -lpspsystemctrl_user -lpspkubridge -lcolordebugger -lgraphics -lansic
#INCDIR = ./include $(ARKROOT)/common/include
INCDIR = $(ARKROOT)/common/include
LDFLAGS =

PSP_EBOOT_TITLE = ARK PRX Injector
PSP_FW_VERSION = 660
BUILD_PRX = 1
EXTRA_TARGETS = EBOOT.PBP

all: $(TARGET).prx

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
