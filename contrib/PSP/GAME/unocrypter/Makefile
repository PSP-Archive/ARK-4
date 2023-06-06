TARGET = unocrypter
OBJS = main.o SysMemUserForUser.o

# Cheap Compilation Workaround if PSPSDK doesn't play along...
# Keep in mind that NID hexediting will be required after compilation!
#OBJS = main.o

LIBS =
CFLAGS = -Os -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS =
#ENCRYPT = 1

PSP_FW_VERSION = 550

BUILD_PRX = 1
PSP_EBOOT_TITLE += UNO Crypter
EBOOT_DISC_ID = NPEH00020

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_ICON = ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
