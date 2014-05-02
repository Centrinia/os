/* vga.c */

#include "util.h"

void
set_vga_register(unsigned short port, unsigned char index,
		 unsigned char value)
{
    /*outb(port, index);
       outb(port+1, value); */
    outportw(port, ((unsigned short) value << 8) | index);
}
