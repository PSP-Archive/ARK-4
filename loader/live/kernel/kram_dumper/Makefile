PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)
LIBS = -L $(ARKROOT)/libs -lgraphics -lcolordebugger -lansic
TARGET = ARK4.BIN

PYTHON=$(shell which python3)

all: pre-build $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) $(LIBS)
CFLAGS += -I $(ARKROOT)/loader/live/kernel/kxploit/ -I $(ARKROOT)/modules/systemctrl/include -I $(ARKROOT)/loader/live/kernel/kernel_loader
CFLAGS += -I $(ARKROOT)/modules/systemctrl/include -DKXPLOIT_LOADADDR=$(KXPLOIT_LOADADDR)

C_OBJS = \
	loader.o \
	$(ARKROOT)/loader/live/kernel/kernel_loader/main.o \
	$(ARKROOT)/core/compat/vitapops/popsdisplay.o \
	$(ARKROOT)/common/utils/functions.o \
	$(ARKROOT)/common/utils/scanner.o \
	$(ARKROOT)/common/utils/patches.o \
	$(ARKROOT)/common/utils/imports.o \
	$(ARKROOT)/common/utils/freemem.o \

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

cmd_genlink = $(PYTHON) $(ARKROOT)/contrib/PC/genlink/genlink.py $(EXPLOIT_LOADADDR) exploitEntry > $(LINKFILE)

link.l:
	$(Q)$(cmd_genlink)

main.elf: $(C_OBJS)

$(TARGET): $(C_OBJS) $(LINKFILE)
	$(Q)$(LD) -T $(LINKFILE) $(C_OBJS) $(LIBS) -o main.elf
	$(Q)$(STRIP) -s main.elf
	$(Q)$(OBJCOPY) -O binary main.elf $@
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf $(TARGET) exploit.h link.l
