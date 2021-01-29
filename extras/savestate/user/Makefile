TARGET = savestate_user
OBJS = \
	start.o \
	main.o \
	imports.o \
	exports.o \
	menucommon.o \
	menumgr.o \
	controller.o \
	ya2d++.o \
	entry.o \
	menu.o \

CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -Wno-unused -Wno-write-strings -Wno-sign-compare
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

INCDIR = libs libs/ya2d
LIBDIR = libs libs/ya2d

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

LIBS = -lstdc++ -lya2d -lpng -ljpeg -lz -lintraFont -lpspmath -lpspgum -lpspgu -lpsprtc -lm -lpspvram

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
