TARGET = bsod_user
OBJS = main.o \
	exports.o \
	imports.o \
	$(ARKROOT)/core/systemctrl/debug/exception_asm.o

INCDIR =  -I $(ARKROOT)/common/include $(ARKROOT)/core/systemctrl/include $(ARKROOT)/libs/graphics
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -std=c99 -DDEBUG=1
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS = -mno-crt0 -nostartfiles
PSP_FW_VERSION=660

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_PSPSDK_LIBS = 1
USE_PSPSDK_LIBC = 1

LIBDIR = $(ARKROOT)/libs
LIBS = -lpspsystemctrl_user -lpspkubridge

all:
	psp-packer $(TARGET).prx
	bin2c $(TARGET).prx $(TARGET).h $(TARGET)

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

clean:
	$(Q)rm -f *.bin *.elf *.prx *.o $(ARKROOT)/core/systemctrl/debug/*.o