PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)
LIBS = -L $(ARKROOT)/libs -lgraphics -lcolordebugger -lsploit -lansic
TARGET = ARKX.BIN

PYTHON=$(shell which python3)

all: pre-build $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR))
CFLAGS += -I $(ARKROOT)/loader/live/kernel/kxploit/ -I $(ARKROOT)/core/systemctrl/include
CFLAGS += -DKXPLOIT_LOADADDR=$(KXPLOIT_LOADADDR)

C_OBJS = \
	main.o \
	../kernel_loader/reboot.o \
	$(ARKROOT)/core/compat/vita/vitaflash.o \
	$(ARKROOT)/core/compat/vitapops/popsdisplay.o \
	$(ARKROOT)/core/systemctrl/src/loadexec.o

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
include $(ARKROOT)/common/make/exploit_config.mak

LINKFILE = link.l

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy

pre-build: link.l

cmd_genlink = $(PYTHON) $(ARKROOT)/contrib/PC/genlink/genlink.py 0x88380000 exploitEntry > $(LINKFILE)

link.l:
	$(Q)$(cmd_genlink)

main.elf: $(C_OBJS)

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $@
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf $(TARGET) exploit.h link.l $(ARKROOT)/core/compat/psp/flashinstall.o
