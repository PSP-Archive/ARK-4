TARGET = ARKMENU
CFLAGS = -Os -G0 -Wall -Wno-unused -Wno-write-strings -Wno-sign-compare
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif
INCDIR = include include/anim include/pmf include/ftp $(ARKROOT)/libs/libya2d $(ARKROOT)/libs/libintraFont $(ARKROOT)/libs/libjpeg $(ARKROOT)/common/include
LIBDIR = $(ARKROOT)/libs
LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = arkMenu
PSP_EBOOT_ICON = ICON0.PNG
PSP_FW_VERSION=660
BUILD_PRX = 1

OBJS = \
		main.o \
		imports.o \
		src/debug.o \
		src/common.o \
		src/system_mgr.o \
		src/gamemgr.o \
		src/net_mgr.o \
		src/entry.o \
		src/iso.o \
		src/umd.o \
		src/eboot.o \
		src/menu.o \
		src/controller.o \
		src/ya2d++.o \
		src/mp3.o \
		src/browser.o \
		src/browser_entries.o \
		src/osk.o \
		src/network.o \
		src/settingsmenu.o \
		src/optionsmenu.o \
		src/texteditor.o \
		src/image_viewer.o \
		src/music_player.o \
		src/anim/anim.o \
		src/anim/pixel.o \
		src/anim/wave.o \
		src/anim/sprites.o \
		src/anim/fire.o \
		src/anim/tetris.o \
		src/anim/matrix.o \
		src/anim/hacker.o \
		src/anim/bsod.o \
		src/anim/snow.o \
		src/anim/gol.o \
		src/anim/noanim.o \
		src/pmf/pmf.o \
		src/pmf/pmf_video.o \
		src/pmf/pmf_decoder.o \
		src/pmf/pmf_audio.o \
		src/pmf/pmf_reader.o \
		src/pmf/at3.o \
		src/ftp/ftp.o \
		src/ftp/ftpd.o \
		src/ftp/sutils.o \
		src/ftp/util.o \
		src/ftp/loadutil.o \
		src/ftp/psp_cfg.o \
		src/ftp/psp_init.o \
		src/ftp/ftpclient.o \
		src/ftp/ftp_driver.o \


LIBS = \
		-lunziprar \
		-lstdc++ \
		-lc \
		-lya2d \
		-lintraFont \
		-ljpeg \
		-lpng \
		-lpspvfpu \
		-lpspmpeg \
		-lpspaudio \
		-lpspatrac3 \
		-lpsppower \
		-lpspgum \
		-lpspgu \
		-lpsprtc \
		-lz \
		-lm \
		-lpspvram \
		-lpspmath \
		-lpspumd \
		-lpspmp3 \
		-lpspwlan \
		-lpspnet \
		-lpspnet_inet \
		-lpsphttp \
		-lpspsystemctrl_user \
		-lpspkubridge \

all: $(TARGET).prx
	$(Q)python3 ./pkg.py

distclean:
	make clean
	$(Q)-rm -f THEME.ARK

arkmenu: all

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
