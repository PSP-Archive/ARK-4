# PSP Software Development Kit - http://www.pspdev.org
# -----------------------------------------------------------------------
# Licensed under the BSD license, see LICENSE in PSPSDK root for details.
#
# build.mak - Base makefile for projects using PSPSDK.
#
# Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
# Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
# Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
#
# $Id: build.mak 355 2005-06-27 07:38:48Z mrbrown $

# Note: The PSPSDK make variable must be defined before this file is included.
ifeq ($(PSPSDK),)
$(error $$(PSPSDK) is undefined.  Use "PSPSDK := $$(shell psp-config --pspsdk-path)" in your Makefile)
endif

CC       = psp-gcc
CXX      = psp-g++
AS       = psp-gcc
LD       = psp-ld
AR       = psp-ar
RANLIB   = psp-ranlib
STRIP    = psp-strip

# Add in PSPSDK includes and libraries.
INCDIR   := $(INCDIR) . $(PSPSDK)/include ../common/
LIBDIR   := $(LIBDIR) . $(PSPSDK)/lib ../common/

CFLAGS   := $(addprefix -I,$(INCDIR)) $(CFLAGS)
CXXFLAGS := $(CFLAGS) $(CXXFLAGS)
ASFLAGS  := $(CFLAGS) $(ASFLAGS)

LDFLAGS  := $(addprefix -L,$(LIBDIR)) $(LDFLAGS)

# Link with following default libraries.  Other libraries should be specified in the $(LIBS) variable.
# TODO: This library list needs to be generated at configure time.
#

FINAL_TARGET = lib$(TARGET).a

all: $(EXTRA_TARGETS) $(FINAL_TARGET)

$(FINAL_TARGET): $(OBJS)
	$(AR) cru $@ $(OBJS)

clean: $(EXTRA_CLEAN)
	-rm -f $(FINAL_TARGET) *.o $(PSP_EBOOT_SFO) $(PSP_EBOOT)
