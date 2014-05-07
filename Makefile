
OBJCOPY = objcopy

AS = gcc 
AFLAGS = -m32 -g

CC = gcc 
COPTS = -O3
CFLAGS = -m32 -g -std=gnu99 -Wall -pedantic 
# Enable stack debugging
CFLAGS += -fno-asynchronous-unwind-tables #-fno-exceptions

KERNEL = kernel.bin
OUTIMAGE = os.img
OBJECTS = boot.o
OBJECTS += e820.o
OBJECTS += mode_switch.o
OBJECTS += protected_mode.o
OBJECTS += isrs.o
OBJECTS += debug.o
OBJECTS += interrupts.o
OBJECTS += text_console.o
OBJECTS += vga.o
OBJECTS += main.o
OBJECTS += paging.o
OBJECTS += util.o
OBJECTS += string.o
OBJECTS += stdlib.o
OBJECTS += serial.o

IMAGE_FILE = out.jpg

all: $(OUTIMAGE)

clean:
	$(RM) $(OUTIMAGE) $(KERNEL) $(OBJECTS) out.raw out.gif kernel.elf kernel.sym

view:
	objdump  -m i8086 -b binary -D $(OUTIMAGE)

.c.o:
	$(CC) $(CFLAGS) $(COPTS) -c $< -o $@
.S.o:
	$(AS) $(AFLAGS) -c $< -o $@

#interrupts.o: interrupts.c
#paging.o: paging.c
#util.o: util.c
#text_console.o: text_console.c
#string.o: string.c
#	$(CC) $(CFLAGS) -O3 -c $< -o $@

$(KERNEL): $(OBJECTS)
	#$(LD) boot.o -o $@ --oformat=binary -Ttext=0x7c00
	# $(LD) $(OBJECTS) -o kernel.elf -Ttext=0x7c00
	$(LD) -melf_i386 $(OBJECTS) -Tkernel.ld -o kernel.elf
	$(OBJCOPY) -O binary kernel.elf $@
	$(OBJCOPY) --only-keep-debug kernel.elf kernel.sym

out.raw: $(IMAGE_FILE)
	convert $(IMAGE_FILE) -resize 640x480\! -colors 256 out.gif
	python output_pixels.py out.gif out.raw

$(OUTIMAGE): $(KERNEL)
	#dd if=/dev/zero of=$(OUTIMAGE) bs=512 count=10
	dd if=/dev/zero of=$(OUTIMAGE) bs=512 count=2880
	dd if=$(KERNEL) of=$(OUTIMAGE) bs=512 seek=0 conv=notrunc

	#dd if=$(KERNEL) of=$(OUTIMAGE) bs=512 seek=0 
	#dd if=out.raw of=$(OUTIMAGE) bs=512 seek=8 conv=notrunc

debug: $(OUTIMAGE)
	qemu -s -S $(OUTIMAGE) &
	gdb -x gdbscript.txt

