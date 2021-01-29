PSPSDK=$(shell psp-config --pspsdk-path)
PROVITA = ../../../
INCDIR = $(PSPSDK)/include $(PROVITA)/common/include $(PROVITA)/libs/graphics
LIBDIR = $(PROVITA)/libs/graphics $(PROVITA)/libs/ansic $(PROVITA)/libs/colordebugger
LIBS = -lgraphics -lansic -lcolordebugger -L $(PROVITA)/libs
TARGET = ARK.BIN

PYTHON=$(shell which python2)

all: $(TARGET)

CFLAGS = -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR))

C_OBJS = \
	main.o \
	$(PROVITA)/common/functions.o

LINKFILE = link.l
CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy

main.elf: $(C_OBJS)

$(TARGET): main.elf
	$(Q)$(LD) -T $(LINKFILE) $(C_OBJS) $(LIBS) -o $<
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf $(TARGET) exploit.h
