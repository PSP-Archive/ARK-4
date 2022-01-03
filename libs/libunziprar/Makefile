TARGET_LIB=libunziprar.a

C_OBJS = \
miniunz.o \
unzip.o \
ioapi.o \
unrar.o \
unrarlib/filestr.o \
unrarlib/recvol.o \
unrarlib/rs.o \
unrarlib/scantree.o \
unrarlib/rar.o \
unrarlib/strlist.o \
unrarlib/strfn.o \
unrarlib/pathfn.o \
unrarlib/int64.o \
unrarlib/savepos.o \
unrarlib/global.o \
unrarlib/file.o \
unrarlib/filefn.o \
unrarlib/filcreat.o \
unrarlib/archive.o \
unrarlib/arcread.o \
unrarlib/unicode.o \
unrarlib/system.o \
unrarlib/isnt.o \
unrarlib/crypt.o \
unrarlib/crc.o \
unrarlib/rawread.o \
unrarlib/encname.o \
unrarlib/resource.o \
unrarlib/match.o \
unrarlib/timefn.o \
unrarlib/rdwrfn.o \
unrarlib/consio.o \
unrarlib/options.o \
unrarlib/ulinks.o \
unrarlib/errhnd.o \
unrarlib/rarvm.o \
unrarlib/rijndael.o \
unrarlib/getbits.o \
unrarlib/sha1.o \
unrarlib/extinfo.o \
unrarlib/extract.o \
unrarlib/volume.o \
unrarlib/list.o \
unrarlib/find.o \
unrarlib/unpack.o \
unrarlib/cmddata.o
OBJS = $(C_OBJS)

all: $(TARGET_LIB)

PSPSDK=$(shell psp-config --pspsdk-path)

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -std=c99 -Wall -Os -G0 -fno-pic -lstdc++
CXXFLAGS = $(CFLAGS) -fexceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =

include $(ARKROOT)/common/make/global.mak
include $(PSPSDK)/lib/build.mak
include $(ARKROOT)/common/make/beauty_lib.mak
