TARGET = unarchive

OBJS = main.o \
	exports.o \
	zip/zip.o \
	zip/inflate.o \
	zip/parse-zip.o \
	zip/uncompress-zip.o \
	rar/rar.o \
	rar/rarvm.o \
	rar/parse-rar.o \
	rar/filter-rar.o \
	rar/huffman-rar.o \
	rar/uncompress-rar.o \
	tar/tar.o \
	tar/parse-tar.o \
	_7z/_7z.o \
	lzmasdk/Bra.o \
	lzmasdk/Bcj2.o \
	lzmasdk/Bra86.o \
	lzmasdk/Delta.o \
	lzmasdk/7zBuf.o \
	lzmasdk/7zDec.o \
	lzmasdk/7zArcIn.o \
	lzmasdk/7zStream.o \
	lzmasdk/CpuArch.o \
	lzmasdk/LzmaDec.o \
	lzmasdk/Lzma2Dec.o \
	lzmasdk/Ppmd7.o \
	lzmasdk/Ppmd8.o \
	lzmasdk/Ppmd7Dec.o \
	lzmasdk/Ppmd8Dec.o \
	lzmasdk/Ppmd7aDec.o \
	common/conv.o \
	common/crc32.o \
	common/custalloc.o \
	common/stream.o \
	common/unarr.o \

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_PSPSDK_LIBS = 1
USE_PSPSDK_LIBC = 1

all:
	psp-packer $(TARGET).prx

INCDIR = $(ARKROOT)/common/include $(PSPSDK)/include common/ zip/ rar/ tar/ lzmasdk/
CFLAGS = -std=c99 -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR = $(ARKROOT)/libs
LIBS = -lpspsysc_user -lpsprtc
LDFLAGS = -mno-crt0 -nostartfiles

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
