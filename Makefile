
OBJCOPY = objcopy

AS = gcc 
AFLAGS = -m32 -g

CC = gcc 
CFLAGS = -m32 -g -std=c99

KERNEL = kernel.bin
OUTIMAGE = os.img
SOURCES = boot.S gdt.S protected_mode.S isrs.S debug.c
OBJECTS = boot.o gdt.o protected_mode.o isrs.o debug.o
IMAGE_FILE = out.jpg

all: $(OUTIMAGE)

clean:
	$(RM) $(OUTIMAGE) $(KERNEL) $(OBJECTS) out.raw out.gif kernel.elf kernel.sym

view:
	objdump  -m i8086 -b binary -D $(OUTIMAGE)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.S.o:
	$(AS) $(AFLAGS) -c $< -o $@

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
	dd if=/dev/zero of=$(OUTIMAGE) bs=512 count=1008
	dd if=$(KERNEL) of=$(OUTIMAGE) bs=512 seek=0 conv=notrunc
	#dd if=out.raw of=$(OUTIMAGE) bs=512 seek=8 conv=notrunc

debug: $(OUTIMAGE)
	qemu -s -S $(OUTIMAGE) &
	gdb -x gdbscript.txt

