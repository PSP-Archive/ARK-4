TARGET = KDUMPER
C_OBJS = main.o imports.o
OBJS = $(C_OBJS)
all: $(TARGET).prx
CFLAGS = -Os -G0 -Wall

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS =
BUILD_PRX = 1

PSP_EBOOT_TITLE += KRamDumper

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
