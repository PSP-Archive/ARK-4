TARGET = psxloader
OBJS = main.o

CFLAGS = -std=c99 -Os -G0 -Wall -I$(ARKROOT)/common/include -I$(ARKROOT)/libs/colordebugger

ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG=1
endif

LDFLAGS = -mno-crt0 -nostartfiles -L. -L $(ARKROOT)/libs
LIBS = -lcolordebugger
PSP_FW_VERSION=660

BUILD_PRX=1

all:
	$(Q)mkpsxiso psxloader.xml

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

clean:
	$(Q)rm *.elf *.prx *.o *.bin *.cue