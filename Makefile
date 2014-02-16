
AS=gcc
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
	$(LD) boot.o -o $@ --oformat=binary -Ttext=0x7c00

$(OUTIMAGE): $(KERNEL)
	dd if=/dev/zero of=$(OUTIMAGE) bs=512 count=256
	dd if=$(KERNEL) of=$(OUTIMAGE) bs=512 seek=0 conv=notrunc


