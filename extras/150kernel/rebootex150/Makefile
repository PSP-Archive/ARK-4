PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/kxploit/ $(ARKROOT)/libs/iplsdk
REBOOTEXDIR = $(ARKROOT)/loader/rebootex
TARGET = rebootex150.bin

PYTHON=$(shell which python3)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) -DMS_IPL

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy
LINKFILE = linkfile.l

OBJS = \
	main.o \

REBOOT_OBJS = \
	lib.o \

LIBS = -L $(ARKROOT)/libs -liplsdk

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lcolordebugger
endif

crt0.o : crt0.S
	$(CC) -I$(PSPSDK)/include -c $< -o crt0.o

$(REBOOT_OBJS) : %.o : $(REBOOTEXDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

main.elf: crt0.o $(REBOOT_OBJS) $(OBJS)

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(TARGET)
	$(Q)bin2c $@ rebootex150.h rebootex150
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf rebootex150.bin rebootex150.h $(EXTRA_CLEAN)

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
