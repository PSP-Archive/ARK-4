TARGET = inferno
C_OBJS = main.o iodrv_funcs.o umd.o isoread.o isocache.o
OBJS = $(C_OBJS) imports.o
all: $(TARGET).prx
INCDIR = $(ARKROOT)/common/include $(ARKROOT)/core/systemctrl/include
CFLAGS = -std=c99 -Os -G0 -Wall

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

LDFLAGS =  -nostartfiles -L $(ARKROOT)/libs
LIBS = -lpspsystemctrl_kernel -lpspsemaphore -lcolordebugger

PSP_FW_VERSION = 660

PRX_EXPORTS = exports.exp

BUILD_PRX=1
USE_KERNEL_LIBS=1
USE_KERNEL_LIBC=1

include $(ARKROOT)/common/make/global.mak
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
