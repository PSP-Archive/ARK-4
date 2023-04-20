PSPSDK=$(shell psp-config --pspsdk-path)
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include ../payloadex
LIBS = -lansic -L $(ARKROOT)/libs
MAINBINEXDIR = $(ARKROOT)/loader/perma/cipl/mainbinex
TARGET = patch.bin

PYTHON=$(shell which python3)

all: $(TARGET)

CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR)) -DMS_IPL

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy
LINKFILE = linkfile.l

MAINBINEX_OBJS = \
	main.o \
	cache.o

$(MAINBINEX_OBJS) : %.o : $(MAINBINEXDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

main.elf: $(MAINBINEX_OBJS) 

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(TARGET)
	$(Q)$(PYTHON) $(ARKROOT)/contrib/PC/timemachine/make_ipl_payload.py --input_payload $(TARGET) --payload_addr 0x040D6000 --ipl_addr 0x040EC000 --output payload.bin 
	$(Q)bin2c payload.bin payload.h ms_ipl_payload
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf patch.bin payload.bin payload.h $(TARGET) $(EXTRA_CLEAN)

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
