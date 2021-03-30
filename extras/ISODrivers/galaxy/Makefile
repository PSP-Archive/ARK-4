ARKROOT = ../../../
K=psp660
TARGET = galaxy
C_OBJS = main.o
OBJS = $(C_OBJS) imports.o $(ARKROOT)/common/functions.o
all: $(TARGET).prx
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/core/systemctrl/include
CFLAGS = -Os -G0 -Wall
LDFLAGS =  -nostartfiles -L. -L$(ARKROOT)/libs

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

PSP_FW_VERSION = 660

BUILD_PRX=1
USE_KERNEL_LIBS=1
USE_KERNEL_LIBC=1

PRX_EXPORTS = export.exp
LIBS += -lpsid -lpspsystemctrl_kernel -lcolordebugger

include $(ARKROOT)/common/make/global.mak
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
