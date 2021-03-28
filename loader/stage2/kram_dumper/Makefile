PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(PROVITA)/common/include $(PROVITA)
LIBS = -L $(PROVITA)/libs -lgraphics -lcolordebugger -lansic
TARGET = ARK4.BIN

PYTHON=$(shell which python2)

all: pre-build $(TARGET)

CFLAGS = -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) $(LIBS)
CFLAGS += -I $(PROVITA)/loader/kxploit/ -I $(PROVITA)/modules/systemctrl/include -I $(PROVITA)/loader/stage2/live
CFLAGS += -I $(PROVITA)/modules/systemctrl/include -DKXPLOIT_LOADADDR=$(KXPLOIT_LOADADDR)

C_OBJS = \
	loader.o \
	$(PROVITA)/common/functions.o \
	$(PROVITA)/loader/stage2/live/main.o \
	$(PROVITA)/core/vitapops/popsdisplay.o \

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

$(TARGET): $(C_OBJS) $(LINKFILE)
	$(Q)$(LD) -T $(LINKFILE) $(C_OBJS) $(LIBS) -o main.elf
	$(Q)$(STRIP) -s main.elf
	$(Q)$(OBJCOPY) -O binary main.elf $@
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf $(TARGET) exploit.h link.l
