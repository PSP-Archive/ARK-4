TARGET          = vshmenu
OBJS            = main.o

CFLAGS          = -Os -G0 -Wall -fshort-wchar
ASFLAGS         = $(CFLAGS)

BUILD_PRX       = 1
PRX_EXPORTS     = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBS            = -lpspsystemctrl_kernel -lpspvshctrl -lpsprtc_driver -lpspreg_driver

PSPSDK          = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
