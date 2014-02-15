
OUTIMAGE = kernel.bin
SOURCES = boot.S
OBJECTS = boot.o

all: $(OUTIMAGE)
clean:
	$(RM) $(OUTIMAGE) $(OBJECTS)

view:
	objdump  -m i8086 -b binary -D $(OUTIMAGE)

.S.o:
	$(AS) -c $< -o $@

$(OUTIMAGE): $(OBJECTS)
	$(LD) boot.o -o $(OUTIMAGE) --oformat=binary -Ttext=0x7c00
