TARGET = LSF
OBJS = main.o

LIBS = -lpspkubridge

BUILD_PRX = 1
PSP_FW_VERSION = 660

CFLAGS = -O2 -G0 -Wall -fshort-wchar
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)


EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = FinalSpeed

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
