SUBDIRS = \
	SystemCtrlForKernel \
	SystemCtrlForUser \
	SysclibForUser \
	KUBridge \
	VshCtrl \
	ansi-c \
	graphics \
	colordebugger \
	iplsdk \
	guglue \
	prebuilt \
	libpspmath \
	libpng \
	libpspvram \
	libya2d \
	libintraFont \
	libpspftp \
	libpspav \
	libvlf \
	libsploit \
	psid 

PREBUILTDIRS = prebuilt libvlf
.PHONY: subdirs $(SUBDIRS)

all: subdirs
	$(Q)cp $(addsuffix /*.a, $(SUBDIRS)) .

distclean clean:
	$(Q)make $@ -C SystemCtrlForKernel $(OPT)
	$(Q)make $@ -C SystemCtrlForUser $(OPT)
	$(Q)make $@ -C VshCtrl $(OPT)
	$(Q)make $@ -C SysclibForUser $(OPT)
	$(Q)make $@ -C KUBridge $(OPT)
	$(Q)make $@ -C ansi-c $(OPT)
	$(Q)make $@ -C graphics $(OPT)
	$(Q)make $@ -C colordebugger $(OPT)
	$(Q)make $@ -C iplsdk $(OPT)
	$(Q)make $@ -C libpspmath $(OPT)
	$(Q)make $@ -C libpng $(OPT)
	$(Q)make $@ -C libpspvram $(OPT)
	$(Q)make $@ -C libya2d $(OPT)
	$(Q)make $@ -C guglue $(OPT)
	$(Q)make $@ -C psid $(OPT)
	$(Q)make $@ -C libintraFont $(OPT)
	$(Q)make $@ -C libpspftp $(OPT)
	$(Q)make $@ -C libpspav $(OPT)
	$(Q)make $@ -C libsploit $(OPT)
	$(Q)rm -rf *.a *~ | true

subdirs: $(SUBDIRS)

$(filter-out $(PREBUILTDIRS), $(SUBDIRS)):
	$(MAKE) $(OPT) -C $@
