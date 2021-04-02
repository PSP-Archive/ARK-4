TARGET = ARK
C_OBJS = main.o $(ARKROOT)/core/compat/psp/flashinstall.o
OBJS = $(C_OBJS)
all: $(TARGET).prx
CFLAGS = -Os -G0 -Wall
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/libs/colordebugger $(ARKROOT)/libs/graphics

LIBS = -lgraphics -lcolordebugger -lansic -lpspvram

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS = -L sysmem -L $(ARKROOT)/libs
BUILD_PRX = 1
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_TITLE += ARK PRO Updater

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
