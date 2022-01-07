TARGET = ingame_menu
OBJS = \
		main.o \
		start.o \
		exitmgr.o \
		imports.o \
		../arkMenu/src/debug.o \
		../arkMenu/src/common.o \
		../arkMenu/src/system_mgr.o \
		../arkMenu/src/controller.o \
		../arkMenu/src/ya2d++.o \
		../arkMenu/src/mp3.o \
		../arkMenu/src/osk.o \
		../arkMenu/src/entry.o \
		../arkMenu/src/eboot.o \
		../arkMenu/src/iso.o \
		../arkMenu/src/cso.o \
		../arkMenu/src/settingsmenu.o \
		../arkMenu/src/optionsMenu.o \
		../arkMenu/src/anim/anim.o \
		../arkMenu/src/anim/pixel.o \
		../arkMenu/src/anim/wave.o \
		../arkMenu/src/anim/sprites.o \
		../arkMenu/src/anim/fire.o \
		../arkMenu/src/anim/tetris.o \
		../arkMenu/src/anim/matrix.o \
		../arkMenu/src/anim/snow.o \
		../arkMenu/src/anim/noanim.o \
#		../arkMenu/src/browser.o \

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
		-lpsppower \
		-lpspgum \
		-lpspgu \
		-lz \
		-lm \
		-lpspvram \
		-lpspmath \
		-lpspmp3 \
		-lpspsystemctrl_user

CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -Wno-unused -Wno-write-strings -Wno-sign-compare
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

INCDIR = ../arkMenu/include ../arkMenu/include/anim $(ARKROOT)/libs/libya2d $(ARKROOT)/libs/libintraFont $(ARKROOT)/libs/libjpeg $(ARKROOT)/common/include
LIBDIR = $(ARKROOT)/libs

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
