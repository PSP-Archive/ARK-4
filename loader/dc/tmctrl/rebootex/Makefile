PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/kxploit/ $(ARKROOT)/libs/iplsdk
REBOOTEXDIR = $(ARKROOT)/loader/rebootex
TARGET = reboot.bin.gz

PYTHON=$(shell which python3)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) -DREBOOTEX -DMS_IPL -DFAT_DEBUG

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
	$(ARKROOT)/common/utils/scanner.o \

LIBS = -L $(ARKROOT)/libs -liplsdk

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lcolordebugger
endif

crt0.o : $(REBOOTEXDIR)/crt0.S
	$(CC) -I$(PSPSDK)/include -c $< -o crt0.o

$(REBOOT_OBJS) : %.o : $(REBOOTEXDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

patches.o: $(ARKROOT)/core/compat/psp/rebootex/patches.c
	$(CC) $(CFLAGS) -I$(REBOOTEXDIR) -c $< -o $@

main.elf: crt0.o $(REBOOT_OBJS) $(C_OBJS) patches.o

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(patsubst %.gz,%,$(TARGET))
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/gz/gz.py $(patsubst %.gz,%,$(TARGET)) $@
	$(Q)bin2c $@ payload.h rebootbuffer_ms_psp
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf reboot.bin reboot.bin.gz payload.h $(EXTRA_CLEAN)

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
