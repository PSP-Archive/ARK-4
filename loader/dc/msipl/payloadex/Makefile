PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/kxploit/ $(ARKROOT)/libs/iplsdk
REBOOTEXDIR = $(ARKROOT)/loader/rebootex
TARGET = ms_payloadex.bin

PYTHON=$(shell which python2)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) -DPAYLOADEX -DMS_IPL

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

$(REBOOT_OBJS) : %.o : $(REBOOTEXDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

patches.o: $(ARKROOT)/core/compat/psp/rebootex/patches.c
	$(CC) $(CFLAGS) -I$(REBOOTEXDIR) -c $< -o $@

main.elf: $(REBOOT_OBJS) $(C_OBJS) patches.o

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(TARGET)
	$(Q)bin2c $@ ms_payloadex.h payloadex
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf ms_payloadex.bin ms_payloadex.h $(EXTRA_CLEAN)

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
