PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(PROVITA)/common/include $(PROVITA)
LIBS = -L $(PROVITA)/libs -lgraphics -lcolordebugger -lansic
TARGET = ark.bin

PYTHON=$(shell which python2)

all: pre-build $(TARGET)

CFLAGS = -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR))
CFLAGS += -I $(PROVITA)/loader/kxploit/ -I $(PROVITA)/modules/systemctrl/include
CFLAGS += -I $(PROVITA)/modules/systemctrl/include -DKXPLOIT_LOADADDR=$(KXPLOIT_LOADADDR) -DFLASH_DUMP=$(FLASH_DUMP)

ifeq ($(FLASH_DUMP), 1)
C_OBJS = \
	main.o \
	flash_dump.o\
	$(PROVITA)/common/functions.o
else
C_OBJS = \
	main.o \
	reboot.o \
	$(PROVITA)/common/functions.o \
	$(PROVITA)/common/flashpatch.o \
	$(PROVITA)/modules/systemctrl/src/loadexec_patch.o
endif

include $(PROVITA)/common/make/global.mak
include $(PROVITA)/common/make/beauty_bin.mak
include $(PROVITA)/common/make/exploit_config.mak

LINKFILE = link.l

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy

pre-build: link.l

cmd_genlink = $(PYTHON) $(PROVITA)/contrib/PC/genlink/genlink.py $(EXPLOIT_LOADADDR) exploitEntry > $(LINKFILE)

link.l:
	$(Q)$(cmd_genlink)

main.elf: $(C_OBJS)

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $@
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf ark.bin exploit.h link.l
