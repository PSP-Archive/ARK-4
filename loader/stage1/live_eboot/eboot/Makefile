TARGET = ARK
C_OBJS = main.o
OBJS = $(C_OBJS)
all: $(TARGET).prx
CFLAGS = -Os -G0 -Wall
PROVITA = ../../../../
INCDIR = $(PSPSDK)/include $(PROVITA)/common/include $(PROVITA)/libs/colordebugger $(PROVITA)/libs/graphics $(PROVITA)/libs/libya2d

LIBS = -lgraphics -lcolordebugger -lansic -lya2d -lpspgu -lpspvram -lpspmath

CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS = -L sysmem -L $(PROVITA)/libs
BUILD_PRX = 1
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_TITLE += ARK Loader

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
