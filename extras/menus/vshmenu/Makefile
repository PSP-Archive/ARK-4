TARGET = satelite
OBJS = \
	main.o \
	blit.o\
  ui.o \
	menu.o \
	advanced.o \
	battery.o \
  vsh.o \
	config.o \
	color.o \
	fonts.o \
	clock.o \
	registry.o \
	umdvideo_list.o \
	vpl.o \
	trans.o \
	trans_en.o \
	stubkk.o \
	launcher.o

BUILD_PRX = 1

INCDIR = $(ARKROOT)/common/include ./include
CFLAGS = -Os -G0 -Wall
ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PRX_EXPORTS = exports.exp

LIBS = -lgraphics -lpspsystemctrl_user -lpspkubridge -lpspvshctrl -lpspreg
LIBDIR = . $(ARKROOT)/libs
LDFLAGS =

PSP_FW_VERSION = 660

vshmenu: all

all:
	psp-packer $(TARGET).prx

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
