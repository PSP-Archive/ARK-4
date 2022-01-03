TARGET_LIB=libjpeg.a

C_OBJS = \
jaricom.o \
jcomapi.o \
jutils.o \
jerror.o \
jmemmgr.o \
jmemnobs.o \
jcapimin.o \
jcapistd.o \
jcarith.o \
jctrans.o \
jcparam.o \
jdatadst.o \
jcinit.o \
jcmaster.o \
jcmarker.o \
jcmainct.o \
jcprepct.o \
jccoefct.o \
jccolor.o \
jcsample.o \
jchuff.o \
jcdctmgr.o \
jfdctfst.o \
jfdctflt.o \
jfdctint.o \
jdapimin.o \
jdapistd.o \
jdarith.o \
jdtrans.o \
jdatasrc.o \
jdmaster.o \
jdinput.o \
jdmarker.o \
jdhuff.o \
jdmainct.o \
jdcoefct.o \
jdpostct.o \
jddctmgr.o \
jidctfst.o \
jidctflt.o \
jidctint.o \
jdsample.o \
jdcolor.o \
jquant1.o \
jquant2.o \
jdmerge.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -std=c99 -O2 -G0
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LIBDIR =
LDFLAGS = -lstdc++

include $(ARKROOT)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty_lib.mak
