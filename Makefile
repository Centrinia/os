
OBJCOPY = objcopy
AS=gcc -g
KERNEL = kernel.bin
OUTIMAGE = os.img
SOURCES = boot.S
OBJECTS = boot.o

all: $(OUTIMAGE)
clean:
	$(RM) $(OUTIMAGE) $(KERNEL) $(OBJECTS)

view:
	objdump  -m i8086 -b binary -D $(OUTIMAGE)

.S.o:
	$(AS) -c $< -o $@

$(KERNEL): $(OBJECTS)
	#$(LD) boot.o -o $@ --oformat=binary -Ttext=0x7c00
	$(LD) boot.o -o kernel.elf -Ttext=0x7c00
	$(OBJCOPY) -O binary kernel.elf $@
	$(OBJCOPY) --only-keep-debug kernel.elf kernel.sym

$(OUTIMAGE): $(KERNEL)
	#dd if=/dev/zero of=$(OUTIMAGE) bs=512 count=10
	dd if=/dev/zero of=$(OUTIMAGE) bs=512 count=1008
	dd if=$(KERNEL) of=$(OUTIMAGE) bs=512 seek=0 conv=notrunc
	dd if=out.raw of=$(OUTIMAGE) bs=512 seek=8 conv=notrunc


