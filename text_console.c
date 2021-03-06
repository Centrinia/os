/* text_console.c */

#include "util.h"
#include "interrupts.h"

void set_frame(int address);
void set_cursor(int address);

struct text_console {
    int xpos;
    int ypos;
    int index;
    int size;
    int width;
    int height;
    int start;
    volatile unsigned short *buffer;
};


struct text_console console;

void clear_console()
{
    for (int i = 0; i < console.size; i++) {
	console.buffer[i] = 0;
    }
}

static inline int mod_console_size(int x)
{
    while (x >= console.size) {
	x -= console.size;
    }
    return x;
}

void print_newline()
{
    console.index += console.width - console.xpos;
    console.xpos = 0;
    console.ypos++;
    for (int i = 0; i < console.width; i++) {
	console.buffer[mod_console_size(i + console.index)] = 0;
	console.buffer[mod_console_size(i + console.index) +
		       console.size] = 0;
    }
    console.start = mod_console_size((console.ypos + 1) * console.width);

    set_frame(console.start);
    set_cursor(mod_console_size(console.index));

    if (console.index >= 2 * console.size) {
	console.index %= 2 * console.size;
    }
}

void show_character(char c, int attribute)
{
    if (c == '\n' || c == '\r') {
	print_newline();
	return;
    }
    const int out = (c & 0xff) | (attribute << 8);
    console.buffer[mod_console_size(console.index)] = out;
    console.buffer[mod_console_size(console.index) + console.size] = out;

    /*console.buffer[console.ypos * console.width + console.xpos] =
       (c & 0xff) | (attribute << 8); */


    console.index++;
    console.xpos++;

    if (console.xpos >= console.width) {
	print_newline();
    }
    //set_cursor(console.index % console.size);
}

void initialize_text_console()
{
    console.xpos = 0;
    console.ypos = 0;
    console.index = 0;
    console.width = 80;
    console.height = 25;
    console.start = 0;
    console.size = console.width * console.height;
    console.buffer = (unsigned short *) 0xb8000;
    clear_console();
}

void set_cursor(int address)
{
    // Address low.
    set_vga_register(0x3d4, 0x0f, address & 0xff);
    // Address high.
    set_vga_register(0x3d4, 0x0e, (address >> 8) & 0xff);
}
