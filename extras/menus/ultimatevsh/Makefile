TARGET          = vshmenu
OBJS            = main.o

ARKROOT ?= $(CURDIR)/../../../
#ARKROOT = /home/brennen/Projects/ARK-4/


INCDIR = $(ARKROOT)/common/include
CFLAGS = -std=c99 -Os -G0 -Wall
#CFLAGS          = -Os -G0 -Wall -fshort-wchar
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS         = $(CFLAGS)

BUILD_PRX       = 1
PRX_EXPORTS     = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = . $(ARKROOT)/libs $(ARKROOT)/libs/KUBridge ./lib ../../xmbctrl/lib
LIBS            = -lpspsystemctrl_kernel -lpspkubridge -lpspvshctrl -lpsprtc_driver -lpspreg_driver

PSPSDK          = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
