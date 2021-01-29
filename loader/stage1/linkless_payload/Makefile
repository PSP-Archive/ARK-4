all: H.BIN

H.BIN:
	psp-as linkless_loader.S
	psp-objcopy -O binary a.out h.bin
	rm a.out

clean:
	rm -rf h.bin

include $(PROVITA)/common/make/global.mak
include $(PROVITA)/common/make/beauty_bin.mak