TARGET = inferno
C_OBJS = main.o iodrv_funcs.o umd.o isoread.o 
OBJS = $(C_OBJS) imports.o
all: $(TARGET).prx
INCDIR = $(PROVITA)/common/include
CFLAGS = -Os -G0 -Wall

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

LDFLAGS =  -nostartfiles -L $(PROVITA)/libs
LIBS = -lpsid -lpspsystemctrl_kernel -lpspsemaphore

PSP_FW_VERSION = 660

PRX_EXPORTS = exports.exp

BUILD_PRX=1
USE_KERNEL_LIBS=1
USE_KERNEL_LIBC=1

include $(PROVITA)/common/make/global.mak
PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty.mak
