CC=arm-linux-gcc
LD=arm-linux-ld
OBJCOPY=arm-linux-objcopy
CFLAGS= -O2 -g
ASFLAGS= -O2 -g
LDFLAGS=-T leeos.lds  -nostartfile -static -nostdlib
SOURCE = start.s init.s abnormal.s boot.c mmu.c interrupt.c list.c mem.c ramdisk.c driver.c  fs.c romfs.c proc.c
BOJS= start.o init.o abnormal.o boot.o mmu.o interrupt.o list.o	mem.o	ramdisk.o driver.o  fs.o romfs.o proc.o

gcc:
	$(CC) $(CFLAGS) -c  $(SOURCE)
	$(LD) $(LDFLAGS)  $(BOJS) -o leeos
	$(OBJCOPY) -O binary  leeos leeos.bin

clean:
	rm *.o leeos leeos.bin -f 


