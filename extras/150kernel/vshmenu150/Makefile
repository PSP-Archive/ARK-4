TARGET = satelite
OBJS = main.o blit.o ui.o menu.o vpl.o trans.o trans_en.o stubkk.o

ARKROOT ?= $(CURDIR)/../../../

BUILD_PRX=1

INCDIR = $(ARKROOT)/common/include
CFLAGS = -std=c99 -Os -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PSP_FW_VERSION = 150

PRX_EXPORTS = exports.exp

LIBS = -lpspsystemctrl_user -lcolordebugger
LIBDIR = . $(ARKROOT)/libs
LDFLAGS = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
