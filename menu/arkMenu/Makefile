TARGET = ARKMENU
CFLAGS = -O2 -G0 -Wall -Wno-unused -Wno-write-strings -Wno-sign-compare
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
INCDIR = include include/anim pmf ftp graphics graphics/ya2d graphics/intraFont graphics/jpeg $(PROVITA)/common/include
LIBDIR = $(PROVITA)/libs
LIBS = -lunziprar -lstdc++ -lc -lya2d -ljpeg -lintraFont -lpng -lpspvfpu -lpspmpeg -lpspaudio -lpspatrac3 -lpspwlan -lpsppower -lpspgum -lpspgu -lz -lm -lpspvram -lpspmath -lpspumd -lpspmp3 -lpspsystemctrl_user
LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = arkMenu
PSP_EBOOT_ICON = ICON0.PNG
PSP_FW_VERSION=371
BUILD_PRX = 1

OBJS = \
		main.o \
		pmf/pmf.o \
		pmf/pmf_video.o \
		pmf/pmf_decoder.o \
		pmf/pmf_audio.o \
		pmf/pmf_reader.o \
		pmf/at3.o \
		ftp/ftp.o \
		ftp/ftpd.o \
		ftp/sutils.o \
		src/debug.o \
		src/common.o \
		src/system_mgr.o \
		src/gamemgr.o \
		src/ftp_mgr.o \
		src/entry.o \
		src/iso.o \
		src/cso.o \
		src/eboot.o \
		src/menu.o \
		src/controller.o \
		src/ya2d++.o \
		src/mp3.o \
		src/zip.o \
		src/browser.o \
		src/osk.o \
		src/plugin.o \
		src/pluginmgr.o \
		src/vshmenu.o \
		src/peops_menu.o \
		src/optionsMenu.o \
		src/anim/anim.o \
		src/anim/pixel.o \
		src/anim/wave.o \
		src/anim/sprites.o \
		src/anim/fire.o \
		src/anim/tetris.o \
		src/anim/matrix.o \
		src/anim/noanim.o

all: $(TARGET).prx
	$(Q)$(PYTHON) ./pkg.py

distclean:
	make clean
	$(Q)-rm -f DATA.PKG

arkmenu: all

include $(PROVITA)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty.mak
