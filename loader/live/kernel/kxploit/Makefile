PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)
LIBS= -L$(ARKROOT)/libs -lcolordebugger -lansic
TARGET = K.BIN

PYTHON=$(shell which python3)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR))

CFLAGS += -I $(ARKROOT)/loader/live/kernel/kxploit/

C_OBJS = main.o $(ARKROOT)/loader/live/kernel/kxploit/$(K)/kxploit.o

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
include $(ARKROOT)/common/make/exploit_config.mak

LINKFILE = link.l

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy

pre-build: link.l

cmd_genlink = $(PYTHON) $(ARKROOT)/contrib/PC/genlink/genlink.py $(KXPLOIT_LOADADDR) initKxploit > $(LINKFILE)

link.l:
	$(Q)$(cmd_genlink)

main.elf: $(C_OBJS)
	$(Q)$(LD) -T $(LINKFILE) $(C_OBJS) -o $(ARKROOT)/loader/live/kernel/kxploit/main.elf $(LIBS)

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $@
	$(Q)mv $(TARGET) $(ARKROOT)/loader/live/kernel/kxploit/$(K)/

kxploits: link.l $(TARGET)

all: clean kxploits

clean:
	$(Q)rm -rf *~ *.s *.o *.elf $(TARGET) exploit.h link.l $(ARKROOT)/loader/live/kernel/kxploit/$(K)/$(TARGET) $(Q)rm $(ARKROOT)/loader/live/kernel/kxploit/$(K)/kxploit.o
