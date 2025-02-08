PSPSDK=$(shell psp-config --pspsdk-path)
LIBS = -lansic -L $(ARKROOT)/libs
MAINBINEXDIR = $(ARKROOT)/loader/perma/cipl/classic/mainbinex
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/loader/rebootex $(MAINBINEXDIR)
TARGET = mainbinex

PYTHON=$(shell which python3)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) -DMS_IPL

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy
LINKFILE = linkfile.l

OBJS = main.o

MAINBINEX_OBJS = \
	cache.o

$(MAINBINEX_OBJS) : %.o : $(MAINBINEXDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

main.elf: $(OBJS) $(MAINBINEX_OBJS) 

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(TARGET)
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf $(TARGET) $(EXTRA_CLEAN)

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
