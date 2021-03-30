TARGET = H.BIN

all: $(TARGET)

$(TARGET):
	psp-as linkless_loader.S
	psp-objcopy -O binary a.out $(TARGET)
	rm a.out

clean:
	rm -rf $(TARGET)

include $(ARKROOT)/common/make/global.mak
include $(ARKROOT)/common/make/beauty_bin.mak
