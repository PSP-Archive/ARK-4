TARGET = ARK
C_OBJS = main.o imports.o
OBJS = $(C_OBJS)
all: $(TARGET).prx
CFLAGS = -std=c99 -Os -G0 -Wall
INCDIR = $(PSPSDK)/include $(ARKROOT)/common/include $(ARKROOT)/libs/colordebugger $(ARKROOT)/libs/graphics

LIBS = -lgraphics -lcolordebugger -lansic -lpspvram -lpspsystemctrl_user

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS = -L sysmem -L $(ARKROOT)/libs
BUILD_PRX = 1
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_TITLE += ARK Loader

EXTRA_TARGETS = EBOOT.PBP

PSP_FW_VERSION = 660

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
