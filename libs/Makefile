SUBDIRS = SystemCtrlForKernel SystemCtrlForUser KUBridge ansi-c graphics colordebugger prebuilt libpspmath libunziprar libpng libjpeg libpspvram libya2d libintraFont psid
.PHONY: subdirs $(SUBDIRS)

all: subdirs
	$(Q)cp $(addsuffix /*.a, $(SUBDIRS)) .

distclean clean:
	$(Q)make $@ -C SystemCtrlForKernel $(OPT)
	$(Q)make $@ -C SystemCtrlForUser $(OPT)
	$(Q)make $@ -C KUBridge $(OPT)
	$(Q)make $@ -C ansi-c $(OPT)
	$(Q)make $@ -C graphics $(OPT)
	$(Q)make $@ -C colordebugger $(OPT)
	$(Q)make $@ -C libpspmath $(OPT)
	$(Q)make $@ -C libunziprar $(OPT)
	$(Q)make $@ -C libpng $(OPT)
	$(Q)make $@ -C libjpeg $(OPT)
	$(Q)make $@ -C libpspvram $(OPT)
	$(Q)make $@ -C libya2d $(OPT)
	$(Q)make $@ -C psid $(OPT)
	$(Q)make $@ -C libintraFont $(OPT)
	$(Q)rm -rf *.a *~ | true

subdirs: $(SUBDIRS)

$(filter-out prebuilt, $(SUBDIRS)):
	$(MAKE) $(OPT) -C $@

prebuilt: ;
