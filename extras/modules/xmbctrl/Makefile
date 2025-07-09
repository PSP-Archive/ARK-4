TARGET = xmbctrl
OBJS = main.o list.o settings.o config.o plugins.o utils.o stub.o exports.o

CFLAGS = -std=c99 -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -std=c99 -I$(ARKROOT)/common/include/
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = include/
LIBDIR = $(ARKROOT)/libs

all:
	psp-packer $(TARGET).prx

USE_KERNEL_LIBS = 0
USE_KERNEL_LIBC = 0
USE_PSPSDK_LIBS = 0
USE_PSPSDK_LIBC = 0

LIBS = -lpspsystemctrl_user -lpspkubridge -lpspvshctrl -lpspreg -lpspsysc_user

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
