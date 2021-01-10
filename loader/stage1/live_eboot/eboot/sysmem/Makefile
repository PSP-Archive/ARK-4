TARGET_LIB = libsysmem_user.a

C_OBJS = SysMemUserForUser.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(PROVITA)/common/include $(PSPSDK)/include
CFLAGS = -G0 -O2
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

include $(PSPSDK)/lib/build.mak
