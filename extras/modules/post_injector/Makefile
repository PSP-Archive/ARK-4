TARGET = injector
OBJS = main.o
CFLAGS = -std=c99 -Os -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif
LIBDIR = $(ARKROOT)/libs
LIBS = -lpspsystemctrl_kernel
INCDIR = ./include $(ARKROOT)/common/include
LDFLAGS =
PRX_EXPORTS = exports.exp

PSP_FW_VERSION = 660
BUILD_PRX = 1

all: $(TARGET).prx

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
include $(ARKROOT)/common/make/beauty.mak
