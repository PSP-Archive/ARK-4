TARGET = systemctrl

C_OBJS = \
	main.o \
	$(PROVITA)/common/functions.o
		
OBJS = \
	$(C_OBJS) \
	$(PROVITA)/modules/systemctrl/src/loadercore.o \
	$(PROVITA)/modules/systemctrl/src/elf.o

all: $(TARGET).prx
INCDIR = $(PROVITA)/common $(PROVITA)
CFLAGS = -Os -G0 -Wall -fno-pic

CFLAGS += -I include -I $(PROVITA)/common/include/ -I $(PROVITA)/libs/graphics/ -I $(PROVITA)/kxploit/ -I $(PROVITA)/modules/systemctrl/include/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(PROVITA)/libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsemaphore -lcolordebugger -lgraphics

PSP_FW_VERSION = 660

include $(PROVITA)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty.mak
