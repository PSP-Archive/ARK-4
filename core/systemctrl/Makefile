TARGET = systemctrl

OBJS = \
	main.o \
	imports.o \
	src/modulemanager.o \
	src/elf.o \
	src/flushcache.o \
	src/loadercore.o \
	src/cryptography.o \
	src/interruptman.o \
	src/kubridge.o \
	src/sctrl.o \
	src/sctrl_se.o \
	src/sctrl_hen.o \
	src/oe_malloc.o \
	src/syspatch.o \
	src/mediasync.o \
	src/hooknids.o \
	src/nidresolver.o \
	src/nid_660_data.o \
	src/missingfunc.o \
	src/rebootex.o \
	src/sysmem.o \
	src/lz4.o \
	src/minilzo.o \
    src/loadexec.o \
	src/exitgame.o \
	src/gameinfo.o \
	src/plugin.o \
	src/leda.o \
	src/psnfix.o \
	src/cpuclock.o   \
	src/msstor_cache.o \
	src/threadman.o \
	src/utility.o \
	src/setlongjmp.o \
	$(ARKROOT)/libs/libsploit/patches.o \
	$(ARKROOT)/libs/ansi-c/lowerstring.o \
	$(ARKROOT)/libs/ansi-c/strcasecmp.o \
	$(ARKROOT)/libs/ansi-c/strsafe.o \


all: $(TARGET).prx
INCDIR = $(ARKROOT)/common $(ARKROOT)
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic

ifdef DISABLE_PSID_CHECK
CFLAGS += -DDISABLE_PSID_CHECK=$(DISABLE_PSID_CHECK)
endif

ifdef FLASH_RESTORE
CFLAGS += -DFLASH_RESTORE=$(FLASH_RESTORE)
endif

CFLAGS += -I include -I $(ARKROOT)/common/include/ -I $(ARKROOT)/libs/graphics/

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(ARKROOT)/libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsemaphore

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
OBJS += debug/exception.o debug/exception_asm.o debug/printk.o debug/jumper.o
LIBS += -lcolordebugger -lgraphics
else
OBJS += debug/dummy.o
endif

PSP_FW_VERSION = 660

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
