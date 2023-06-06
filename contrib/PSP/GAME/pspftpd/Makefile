TARGET = psp-ftpd
APPOBJS= psp_main.o util.o loadutil.o ftpd.o ftp.o sutils.o psp_init.o psp_cfg.o psp_pg.o psp_font.o

OBJS = $(APPOBJS) 
LIBS=-lpspwlan -lpsppower

PSP_FW_VERSION=660

INCDIR = 
CFLAGS = -O2 -Wall \
 -funroll-loops -ffast-math -fomit-frame-pointer \
 -fno-strength-reduce -finline-functions \
 -fsingle-precision-constant

ASFLAGS = $(CFLAGS) -c
 
LIBDIR =
LDFLAGS =
BUILD_PRX=1

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = FTP for Vita
PSP_EBOOT_ICON= ICON0.PNG

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
