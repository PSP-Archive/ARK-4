TARGET=unziprar

OBJS = main.o \
	exports.o \
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

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_PSPSDK_LIBC = 1
USE_PSPSDK_LIBS = 1

all:
	psp-packer $(TARGET).prx

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-use-cxa-atexit
ASFLAGS = $(CFLAGS)
LIBDIR =
LIBS = -lstdc++ -lz -lc
LDFLAGS =

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
