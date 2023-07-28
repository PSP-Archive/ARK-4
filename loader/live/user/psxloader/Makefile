TARGET = psxloader
OBJS = main.o

CFLAGS = -std=c99 -Os -G0 -Wall -I$(ARKROOT)/common/include 

ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG=1
endif

LDFLAGS = -mno-crt0 -nostartfiles -L. -L $(ARKROOT)/libs
LIBS = 
PSP_FW_VERSION=660

BUILD_PRX=1

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

#ChovySign-CLI --pops psxloader.cue --pops-info "ARK-X" ICON0.PNG --pops-eboot psxloader.prx --no-psvimg --vkey-gen act.dat license.rif 000000000000000000000000 2 --rif license.rif
psxloader.cue:
	$(Q)mkpsxiso psxloader.xml

main.c: psxloader.cue

clean:
	$(Q)rm -f *.elf *.prx *.o *.bin *.cue
