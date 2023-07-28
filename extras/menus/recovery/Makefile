TARGET = ARKRECOVERY
CFLAGS = -O3 -G0 -Wall -Wno-unused -Wno-write-strings -Wno-sign-compare
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
INCDIR = ./include ../arkMenu/include ../arkMenu/include/anim ../arkMenu/include/multimedia $(ARKROOT)/libs/libya2d $(ARKROOT)/libs/libintraFont $(ARKROOT)/libs/libjpeg $(ARKROOT)/common/include
LIBDIR = $(ARKROOT)/libs
LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = ARK Recovery
PSP_EBOOT_ICON = ../arkMenu/ICON0.PNG
PSP_FW_VERSION=660
BUILD_PRX = 1

OBJS = \
		main.o \
		../arkMenu/src/debug.o \
		../arkMenu/src/common.o \
		../arkMenu/src/lang.o \
		../arkMenu/src/cJSON.o \
		../arkMenu/src/system_mgr.o \
		../arkMenu/src/controller.o \
		../arkMenu/src/ya2d++.o \
		../arkMenu/src/osk.o \
		../arkMenu/src/entry.o \
		../arkMenu/src/eboot.o \
		../arkMenu/src/iso.o \
		../arkMenu/src/usb.o \
		../arkMenu/src/browser.o \
		../arkMenu/src/browser_entries.o \
		../arkMenu/src/settingsmenu.o \
		../arkMenu/src/optionsmenu.o \
		../arkMenu/src/texteditor.o \
		../arkMenu/src/image_viewer.o \
		../arkMenu/src/music_player.o \
		../arkMenu/src/anim/anim.o \
		../arkMenu/src/anim/pixel.o \
		../arkMenu/src/anim/wave.o \
		../arkMenu/src/anim/sprites.o \
		../arkMenu/src/anim/fire.o \
		../arkMenu/src/anim/tetris.o \
		../arkMenu/src/anim/matrix.o \
		../arkMenu/src/anim/hacker.o \
		../arkMenu/src/anim/bsod.o \
		../arkMenu/src/anim/snow.o \
		../arkMenu/src/anim/gol.o \
		../arkMenu/src/anim/noanim.o \
		../arkMenu/src/multimedia/mp3.o \
		../arkMenu/src/multimedia/mpeg.o \
		../arkMenu/src/multimedia/mpeg_video.o \
		../arkMenu/src/multimedia/mpeg_decoder.o \
		../arkMenu/src/multimedia/mpeg_audio.o \
		../arkMenu/src/multimedia/mpeg_reader.o \
		../arkMenu/src/multimedia/at3.o \

LIBS = \
		-lunziprar \
		-lstdc++ \
		-lc \
		-lya2d \
		-lintraFont \
		-ljpeg \
		-lpng \
		-lpspvfpu \
		-lpspaudio \
		-lpspmpeg \
		-lpspatrac3 \
		-lpsppower \
		-lpspgum \
		-lpspgu \
		-lpsprtc \
		-lz \
		-lm \
		-lpspusb \
		-lpspusbdevice_driver \
		-lpspvram \
		-lpspmath \
		-lpspmp3 \
		-lpspumd \
		-lpspdisplay \
		-lpspsystemctrl_user \
		-lpspkubridge \

all: $(TARGET).prx

distclean:
	make clean

recovery: all

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
