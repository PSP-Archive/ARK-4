TARGET = ipl_stage2_payload
EXTRA_TARGETS = ipl_stage2_payload.h
OBJS = crt0.o main.o

INCDIR = $(ARKROOT)/loader/rebootex $(ARKROOT)/libs/iplsdk
CFLAGS = -Os -G0 -Wall -std=c99 -fno-pic $(BFLAGS) -fdata-sections -ffunction-sections -Wl,--gc-sections
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti 
ASFLAGS = $(CFLAGS)

LIBS = -liplsdk
LIBDIR = $(ARKROOT)/libs
LDFLAGS = -nostartfiles  -T linkfile.l -fdata-sections -ffunction-sections -Wl,--gc-sections

ipl_stage2_payload.h: $(TARGET).bin
	$(Q)bin2c $(TARGET).bin ipl_stage2_payload.h payload

PSPSDK=$(shell psp-config --pspsdk-path)
include $(ARKROOT)/common/make/build_ipl.mak