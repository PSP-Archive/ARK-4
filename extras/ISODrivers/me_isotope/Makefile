release: all
	psp-packer $(TARGET).prx
	bin2c $(TARGET).prx $(TARGET)_bin.h $(TARGET)
	sed -i "/int/d" $(TARGET)_bin.h
	rm -f *.elf *.o

ifndef $(TARGET_FW)
TARGET_FW = 660
endif


TARGET = isotope
OBJS = main.o sceumd.o sceumdman.o umd9660_march_driver.o isoread.o csoread.o  stub.o exports$(TARGET_FW).o


PSP_FW_VERSION=$(TARGET_FW)

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -D_PSP_FW_VERSION=$(TARGET_FW)

BUILD_PRX = 1
PRX_EXPORTS = exports$(TARGET_FW).exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1


LDFLAGS += -mno-crt0 -nostartfiles
LIBDIR = ../lib
LIBS = -lpspsystemctrl_kernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
