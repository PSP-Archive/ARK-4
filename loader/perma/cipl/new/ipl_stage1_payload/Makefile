TARGET = ipl_stage1_payload
OBJS = crt0.o main.o

INCDIR = ../ipl_stage2_payload $(ARKROOT)/common/include $(ARKROOT)/libs/iplsdk
CFLAGS = -Os -G0 -Wall -std=c99 -fno-pic $(BFLAGS) -fdata-sections -ffunction-sections -Wl,--gc-sections
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti 
ASFLAGS = $(CFLAGS)

LIBS = -liplsdk
LIBDIR = $(ARKROOT)/libs
LDFLAGS = -nostartfiles  -T linkfile.l -fdata-sections -ffunction-sections -Wl,--gc-sections

PSPSDK=$(shell psp-config --pspsdk-path)
include $(ARKROOT)/common/make/build_ipl.mak
