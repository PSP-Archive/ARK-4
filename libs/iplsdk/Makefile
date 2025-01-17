TARGET = iplsdk
OBJS = \
	cache.o \
	cpu.o \
	display.o \
	dmacplus.o \
	emcddr.o \
	chip.o \
	ecc.o \
	emcsm.o \
	scramble.o \
	state.o \
	exception.o \
	exception_asm.o \
	gpio.o \
	interrupt.o \
	interrupt_asm.o \
	kirk.o \
	lcd.o \
	phatlcd.o \
	hibari.o \
	samantha.o \
	streetlcd.o \
	tmdlcd.o \
	lcdc.o \
	led.o \
	lflash.o \
	model.o \
	mspro.o \
	pwm.o \
	spi.o \
	comms.o \
	handshake.o \
	syscon.o \
	sysreg.o \
	uart.o \
	delay.o \
	fat.o \
	utils.o


INCDIR = 
CFLAGS = -Os -G0 -Wall -fno-pic -std=c99 $(BFLAGS) -fdata-sections -ffunction-sections -Wl,--gc-sections
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti 
ASFLAGS = $(CFLAGS)

LIBS =
LIBDIR = 
LDFLAGS = 


PSPSDK=$(shell psp-config --pspsdk-path)
include $(ARKROOT)/libs/iplsdk/build_lib.mak