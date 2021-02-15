TARGET = satelite
OBJS = main.o blit.o ui.o menu.o vpl.o trans.o trans_en.o stubkk.o

PROVITA ?= $(CURDIR)/../../../

BUILD_PRX=1

INCDIR = $(PROVITA)/common/include
CFLAGS = -Os -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PRX_EXPORTS = exports.exp

LIBS = -lpspsystemctrl_user -lpspkubridge
LIBDIR = . $(PROVITA)/libs
LDFLAGS =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
