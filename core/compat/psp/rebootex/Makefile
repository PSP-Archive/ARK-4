PSPSDK=$(shell psp-config --pspsdk-path)
REBOOTEX_DIR = $(ARKROOT)/loader/rebootex
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/kxploit/ $(REBOOTEX_DIR)
TARGET = reboot.bin.gz

PYTHON=$(shell which python2)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) -DREBOOTEX

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy
LINKFILE = linkfile.l

C_OBJS = \
	patches.o \
	$(REBOOTEX_DIR)/main.o \
	$(REBOOTEX_DIR)/lib.o \
	$(REBOOTEX_DIR)/pspbtcnf.o \
	$(ARKROOT)/common/utils/scanner.o \

LIBS = -L $(ARKROOT)/libs -lcolordebugger

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lcolordebugger
endif

main.elf: $(C_OBJS)
	$(LD) -o main.elf $(C_OBJS)

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(patsubst %.gz,%,$(TARGET))
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/gz/gz.py $(patsubst %.gz,%,$(TARGET)) $@
	$(Q)bin2c $@ payload.h rebootbuffer_psp
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf reboot.bin reboot.bin.gz payload.h $(EXTRA_CLEAN) $(REBOOTEX_DIR)/*.o

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
