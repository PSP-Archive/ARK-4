TARGET = systemctrl

C_OBJS = \
	src/main.o \
	src/modulemanager.o \
	src/elf.o \
	src/flushcache.o \
	src/loadercore.o \
	src/cryptography.o \
	src/interruptman.o \
	src/kubridge.o \
	src/sctrl.o \
	src/sctrl_se.o \
	src/oe_malloc.o \
	src/syspatch.o \
	src/mediasync.o \
	src/msstor_cache.o \
	src/hooknids.o \
	src/nidresolver.o \
	src/nid_660_data.o \
	src/missingfunc.o \
	src/plugin.o \
	src/rebootex.o \
	src/sysmem.o \
	src/vlffix.o \
	src/exception.o \
	src/printk.o \
	src/jumper.o \
	src/threadctx.o \
	src/lz4.o \
	src/loadexec_patch.o \
	$(PROVITA)/core/vitacompat/exitgame.o \
	$(PROVITA)/common/functions.o
		
OBJS = \
	$(C_OBJS) \
	exception_asm.o \
	setlongjmp.o
all: $(TARGET).prx
INCDIR = $(PROVITA)/common $(PROVITA)
CFLAGS = -Os -G0 -Wall -fno-pic

ifdef DISABLE_PSID_CHECK
CFLAGS += -DDISABLE_PSID_CHECK=$(DISABLE_PSID_CHECK)
endif

ifdef FLASH_RESTORE
CFLAGS += -DFLASH_RESTORE=$(FLASH_RESTORE)
endif

CFLAGS += -I include -I $(PROVITA)/common/include/ -I $(PROVITA)/libs/graphics/

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = $(PROVITA)/libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsemaphore -lgraphics -lcolordebugger -lpsppower

PSP_FW_VERSION = 660

include $(PROVITA)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty.mak
