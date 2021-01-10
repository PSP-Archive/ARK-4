all:
	../contrib/pspgz.py vshctrl.prx ../contrib/SystemControl.hdr vshctrl.prx VshCtrl 0x1007

TARGET = vshctrl
OBJS = main.o vshpatch.o custom_update.o xmbiso.o isoreader.o virtual_pbp.o vshmenu.o usbdevice.o dirent_track.o vshctrl_patch_offset.o disctype.o hibernation_delete.o ../Common/utils.o ../Common/libs.o ../Common/strsafe.o
IMPORTS =

INCDIR = ../Common/ ../include/
CFLAGS = -Os -G0 -Wall -fno-pic

ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG=1
OBJS += ../Common/printk.o
endif

ifeq ($(NIGHTLY), 1)
CFLAGS += -DNIGHTLY=\"$(shell hg summary | grep parent | cut -d " " -f 2)\"
endif

ifeq ($(CONFIG_620), 1)
CFLAGS += -DCONFIG_620=1
PSP_FW_VERSION = 620
endif

ifeq ($(CONFIG_635), 1)
CFLAGS += -DCONFIG_635=1
PSP_FW_VERSION = 635
endif

ifeq ($(CONFIG_639), 1)
CFLAGS += -DCONFIG_639=1
PSP_FW_VERSION = 639
endif

ifeq ($(CONFIG_660), 1)
CFLAGS += -DCONFIG_660=1
PSP_FW_VERSION = 660
endif

ifeq ($(CONFIG_661), 1)
CFLAGS += -DCONFIG_661=1
PSP_FW_VERSION = 661
endif

OBJS += $(IMPORTS)

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = ../libs
LDFLAGS =  -nostartfiles
LIBS = -lpspsystemctrl_kernel -lpspusb -lpspusbdevice_driver

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
