PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/libs/graphics
LIBDIR = $(ARKROOT)/libs/graphics $(ARKROOT)/libs/ansic $(ARKROOT)/libs/colordebugger
LIBS = -lgraphics -lansic -lcolordebugger -L $(ARKROOT)/libs
TARGET = ARK.BIN

PYTHON=$(shell which python3)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR))

C_OBJS = \
	main.o \
	$(ARKROOT)/common/utils/scanner.o \

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
