PSPSDK=$(shell psp-config --pspsdk-path)
PSPDEV=$(shell psp-config --pspdev-path)
INCLUDE=$(PSPSDK)/include

all:    main

clean:
	rm -rf *~ *.o *.elf *.bin main.s *.bin 

CC       = psp-gcc
CFLAGS   := -D PSP -I $(INCLUDE) -W -Wall -O2 -G0 -fno-pic -mno-abicalls -w -fomit-frame-pointer

ASM      = psp-as

sdk.o: sdk.S
	$(ASM) config/sdk.S -o config/sdk.o

main.o: main.c
	$(CC) $(CFLAGS) -S main.c -o main.s
	$(ASM) main.s -o main.o

graphics.o: graphics.c
	$(CC) $(CFLAGS) -S graphics/graphics.c -o graphics/graphics.s
	$(ASM) graphics/graphics.s -o graphics/graphics.o

common.o: common.c
	$(CC) $(CFLAGS) -S common/common.c -o common/common.s
	$(ASM) common/common.s -o common/common.o

rand.o: rand.c
	$(CC) $(CFLAGS) -S common/rand.c -o common/rand.s
	$(ASM) common/rand.s -o common/rand.o

snow.o: snow.c
	$(CC) $(CFLAGS) -S anim/snow.c -o anim/snow.s
	$(ASM) anim/snow.s -o anim/snow.o

main: main.o config/sdk.o graphics/graphics.o common/common.o common/rand.o anim/snow.o linker.x
	$(PSPDEV)/bin/psp-ld -T linker.x -L$(PSPSDK)/lib main.o config/sdk.o graphics/graphics.o common/common.o common/rand.o anim/snow.o -o main.elf
	$(PSPDEV)/bin/psp-strip -s main.elf
	$(PSPDEV)/bin/psp-objcopy -O binary main.elf h.bin


