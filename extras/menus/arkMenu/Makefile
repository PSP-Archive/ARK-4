TARGET = ARKMENU
CFLAGS = -Os -G0 -Wall -Wno-unused -Wno-write-strings -Wno-sign-compare
CXXFLAGS = $(CFLAGS) -fexceptions -fno-rtti
ASFLAGS = $(CFLAGS)
INCDIR = include include/anim include/pmf $(ARKROOT)/libs/libya2d $(ARKROOT)/libs/libintraFont $(ARKROOT)/libs/libjpeg $(ARKROOT)/common/include
LIBDIR = $(ARKROOT)/libs
LIBS = -lunziprar -lstdc++ -lc -lya2d -lintraFont -ljpeg -lpng -lpspvfpu -lpspmpeg -lpspaudio -lpspatrac3 -lpsppower -lpspgum -lpspgu -lz -lm -lpspvram -lpspmath -lpspumd -lpspmp3 -lpspsystemctrl_user
LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = arkMenu
PSP_EBOOT_ICON = ICON0.PNG
PSP_FW_VERSION=371
BUILD_PRX = 1

OBJS = \
		main.o \
		src/debug.o \
		src/common.o \
		src/system_mgr.o \
		src/gamemgr.o \
		src/entry.o \
		src/iso.o \
		src/cso.o \
		src/eboot.o \
		src/menu.o \
		src/controller.o \
		src/ya2d++.o \
		src/mp3.o \
		src/browser.o \
		src/osk.o \
		src/settingsmenu.o \
		src/optionsMenu.o \
		src/anim/anim.o \
		src/anim/pixel.o \
		src/anim/wave.o \
		src/anim/sprites.o \
		src/anim/fire.o \
		src/anim/tetris.o \
		src/anim/matrix.o \
		src/anim/noanim.o \
		src/pmf/pmf.o \
		src/pmf/pmf_video.o \
		src/pmf/pmf_decoder.o \
		src/pmf/pmf_audio.o \
		src/pmf/pmf_reader.o \
		src/pmf/at3.o \

all: $(TARGET).prx
	$(Q)python3 ./pkg.py

distclean:
	make clean
	$(Q)-rm -f DATA.PKG

arkmenu: all

include $(ARKROOT)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty.mak
