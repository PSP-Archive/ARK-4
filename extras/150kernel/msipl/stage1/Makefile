PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/libs/iplsdk
REBOOTEXDIR = $(ARKROOT)/loader/rebootex
TARGET = msipl

PYTHON=$(shell which python3)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) -DMS_IPL

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy
LINKFILE = linkfile.l

REBOOT_OBJS = \
	lib.o \

C_OBJS = \
	crt0.o \
	main.o \

LIBS = -L $(ARKROOT)/libs -liplsdk

$(REBOOT_OBJS) : %.o : $(REBOOTEXDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

crt0.o : crt0.S
	$(CC) -I$(PSPSDK)/include -c $< -o crt0.o

main.elf: $(REBOOT_OBJS) $(C_OBJS)

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(TARGET)
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf msipl $(EXTRA_CLEAN)

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
