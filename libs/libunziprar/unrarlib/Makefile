TARGET = rar
#OBJS = arccmt.o archive.o arcread.o beosea.o cmddata.o coder.o consio.o crc.o crypt.o dll.o encname.o errhnd.o extinfo.o extract.o filcreat.o file.o filefn.o filestr.o find.o getbits.o global.o int64.o isnt.o list.o log.o match.o model.o options.o os2ea.o pathfn.o rar.o rarvm.o rarvmtbl.o rawread.o rdwrfn.o recvol.o resource.o rijndael.o rs.o savepos.o scantree.o sha1.o smallfn.o strfn.o strlist.o suballoc.o system.o timefn.o ulinks.o unicode.o unios2.o unpack15.o unpack20.o unpack.o uowners.o volume.o

COMPILE=$(CXX) $(CXXFLAGS) $(DEFINES)
LINK=$(CXX)

UNRAR_OBJ=filestr.o recvol.o rs.o scantree.o
LIB_OBJ=filestr.o scantree.o dll.o

OBJS=filestr.o recvol.o rs.o scantree.o rar.o strlist.o strfn.o pathfn.o int64.o savepos.o global.o file.o filefn.o filcreat.o \
	archive.o arcread.o unicode.o system.o isnt.o crypt.o crc.o rawread.o encname.o \
	resource.o match.o timefn.o rdwrfn.o consio.o options.o ulinks.o errhnd.o rarvm.o \
	rijndael.o getbits.o sha1.o extinfo.o extract.o volume.o list.o find.o unpack.o cmddata.o

INCDIR = "/home/Paul/pspgames/freetype/include"
CXX=g++
CXXFLAGS = -O2 -G0 -Wall -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lpspdebug -lpspsdk -lstdc++ -lm
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = AnTi Unzip

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

	
