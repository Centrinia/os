ENTRY(_start);
SECTIONS
{
    . = 0x7C00;
    .start : AT(0x7C00)
    {
       _text = .;
        boot.o(.text);
        _text_end = .;
    }
    .sig : AT(0x7DFE)
    {
        SHORT(0xaa55);
    }
    . = 0x7E00;
    .text : AT(0x7E00)
    {
        gdt.o(.text);
        protected_mode.o(.text);
        isrs.o(.text);
    }

    .data :
    {
        _data = .;
        *(.bss);
        *(.bss*);
        *(.data);
        *(.rodata*);
        *(COMMON)
        _data_end = .;
    }
    /DISCARD/ :
    {
        *(.note*);
        *(.iplt*);
        *(.igot*);
        *(.rel*);
        *(.comment);
/* add any unwanted sections spewed out by your version of gcc and flags here */
    }
}
