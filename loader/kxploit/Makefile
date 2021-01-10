PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(PROVITA)/common/include $(PROVITA)
LIBS= -L$(PROVITA)/libs -lcolordebugger -lansic
TARGET = k.bin

PYTHON=$(shell which python2)

all: pre-build $(TARGET)

CFLAGS = -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR))

CFLAGS += -I $(PROVITA)/loader/kxploit/

C_OBJS = main.o $(PROVITA)/loader/kxploit/$(K)/kxploit.o $(PROVITA)/common/functions.o $(PROVITA)/common/flashpatch.o

include $(PROVITA)/common/make/global.mak
include $(PROVITA)/common/make/beauty_bin.mak
include $(PROVITA)/common/make/exploit_config.mak

LINKFILE = link.l

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy

pre-build: link.l

cmd_genlink = $(PYTHON) $(PROVITA)/contrib/PC/genlink/genlink.py $(KXPLOIT_LOADADDR) initKxploit > $(LINKFILE)

link.l:
	$(Q)$(cmd_genlink)

main.elf: $(C_OBJS)
	$(Q)$(LD) -T $(LINKFILE) $(C_OBJS) -o $(PROVITA)/loader/kxploit/main.elf $(LIBS)

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $@
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf k.bin exploit.h link.l $(PROVITA)/loader/kxploit/$(K)/kxploit.o
