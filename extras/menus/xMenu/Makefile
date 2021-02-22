TARGET = XMENU
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
INCDIR = ../../libs/libpng ../../libs/libintraFont
LIBDIR = ../../libs
LIBS = -lstdc++ -lpng -lintraFont -lpspgu -lpspgum -lz -lm -lpspsystemctrl_user
LDFLAGS = -L libs
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = xMenu
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_PIC1 = PIC1.PNG

OBJS = \
		main.o \
		graphics/font.o \
		graphics/framebuffer.o \
		graphics/graphics.o \
		xmenu/debug.o \
		xmenu/common.o \
		xmenu/menu.o \
		xmenu/entry.o \
		xmenu/controller.o \
		xmenu/text.o


all: $(TARGET).prx
distclean: clean

xmenu: all

include $(PROVITA)/common/make/global.mak
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
include $(PROVITA)/common/make/beauty.mak
