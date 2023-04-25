PSPSDK=$(shell psp-config --pspsdk-path)
REBOOTEXDIR = $(ARKROOT)/loader/rebootex
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/kxploit/ $(REBOOTEXDIR)
TARGET = reboot.bin.gz

PYTHON=$(shell which python3)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) -DREBOOTEX

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy
LINKFILE = $(REBOOTEXDIR)/linkfile.l

REBOOT_OBJS = \
	main.o \
	lib.o \
	pspbtcnf.o \

C_OBJS = \
	patches.o \
	$(ARKROOT)/common/utils/scanner.o \

LIBS = -L $(ARKROOT)/libs

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lcolordebugger
endif

$(REBOOT_OBJS) : %.o : $(REBOOTEXDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

crt0.o : $(REBOOTEXDIR)/crt0.S
	$(CC) -I$(PSPSDK)/include -c $< -o crt0.o

main.elf: crt0.o $(C_OBJS) $(REBOOT_OBJS)

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(patsubst %.gz,%,$(TARGET))
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/gz/gz.py $(patsubst %.gz,%,$(TARGET)) $@
	$(Q)bin2c $@ payload.h rebootbuffer_pentazemin
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf reboot.bin reboot.bin.gz payload.h $(EXTRA_CLEAN)

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
