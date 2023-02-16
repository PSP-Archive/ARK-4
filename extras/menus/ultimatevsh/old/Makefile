TARGET          = VSHMENU
OBJS            = main.o

ARKROOT ?= $(CURDIR)/../../../


INCDIR = $(ARKROOT)/common/include
CFLAGS = -std=c99 -Os -G0 -Wall
#CFLAGS          = -Os -G0 -Wall -fshort-wchar
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS         = $(CFLAGS)
ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

PSP_FW_VERSION = 500
BUILD_PRX       = 1
PRX_EXPORTS     = exports.exp

#USE_KERNEL_LIBC = 1
#USE_KERNEL_LIBS = 1

LIBDIR = $(ARKROOT)/libs ./lib #../../xmbctrl/lib #$(ARKROOT)/libs/KUBridge 
LIBS            = -lpspsystemctrl_kernel -lpspsystemctrl_user -lpspdisplay -lpsprtc_driver -lpspreg_driver #-lpspvshctrl #-lpspkubridge    

PSPSDK          = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
