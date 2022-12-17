PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/kxploit/
REBOOTEXDIR = $(ARKROOT)/loader/rebootex
TARGET = nand_payloadex.bin

PYTHON=$(shell which python2)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) -DPAYLOADEX

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy
LINKFILE = $(REBOOTEXDIR)/linkfile.l

REBOOT_OBJS = \
	main.o \
	lib.o \
	pspbtcnf.o \
	psp_patches.o \

C_OBJS = \
	$(ARKROOT)/common/utils/scanner.o

LIBS = -L $(ARKROOT)/libs -lcolordebugger

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lcolordebugger
endif

$(REBOOT_OBJS) : %.o : $(REBOOTEXDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

main.elf: $(REBOOT_OBJS) $(C_OBJS) 

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(TARGET)
	$(Q)bin2c $@ nand_payloadex.h payloadex
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf nand_payloadex.bin nand_payloadex.h $(EXTRA_CLEAN)

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
