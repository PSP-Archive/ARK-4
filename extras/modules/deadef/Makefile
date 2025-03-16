TARGET = deadef
OBJS = main.o exports.o

INCDIR =  -I $(ARKROOT)/common/include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -std=c99
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION=660

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

all:
	psp-packer $(TARGET).prx
	$(PYTHON) $(ARKROOT)/contrib/PC/btcnf/btcnf.py build pspbtcnf_05g_deaf.txt

LIBDIR = $(ARKROOT)/libs
LIBS = -lpspsystemctrl_kernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
