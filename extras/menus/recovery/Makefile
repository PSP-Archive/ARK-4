TARGET = ARKMENU
CFLAGS = -O2 -G0 -Wall -Wno-unused -Wno-write-strings -Wno-sign-compare
CXXFLAGS = $(CFLAGS) -fexceptions -fno-rtti
ASFLAGS = $(CFLAGS)
INCDIR = ../arkMenu/include ../arkMenu/include/anim ../arkMenu/graphics ../arkMenu/graphics/ya2d ../arkMenu/graphics/intraFont ../arkMenu/graphics/jpeg $(PROVITA)/common/include
LIBDIR = $(PROVITA)/libs
LIBS = -lunziprar -lstdc++ -lc -lya2d -lintraFont -ljpeg -lpng -lpspvfpu -lpspmpeg -lpspaudio -lpspatrac3 -lpspwlan -lpsppower -lpspgum -lpspgu -lz -lm -lpspvram -lpspmath -lpspumd -lpspmp3 -lpspsystemctrl_user
LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = ARK Recovery
PSP_EBOOT_ICON = ../arkMenu/ICON0.PNG
PSP_FW_VERSION=371
BUILD_PRX = 1

OBJS = \
		main.o \
		../arkMenu/src/debug.o \
		../arkMenu/src/common.o \
		../arkMenu/src/system_mgr.o \
		../arkMenu/src/controller.o \
		../arkMenu/src/ya2d++.o \
		../arkMenu/src/mp3.o \
		../arkMenu/src/vshmenu.o \
		../arkMenu/src/optionsMenu.o \
		../arkMenu/src/anim/anim.o \
		../arkMenu/src/anim/pixel.o \
		../arkMenu/src/anim/wave.o \
		../arkMenu/src/anim/sprites.o \
		../arkMenu/src/anim/fire.o \
		../arkMenu/src/anim/tetris.o \
		../arkMenu/src/anim/matrix.o \
		../arkMenu/src/anim/noanim.o \

all: $(TARGET).prx

distclean:
	make clean

recovery: all

include $(PROVITA)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty.mak
