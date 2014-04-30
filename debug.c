/* debug.c */

void set_frame(int address);

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

void print_newline()
{
    console.index += console.width - console.xpos;
    console.xpos = 0;
    console.ypos++;
    for (int i = 0; i < console.width; i++) {
	//console.buffer[(i+console.index) % console.size] = 0;
	console.buffer[(i + console.index) % console.size + console.size] =
	    0;
    }
    console.start = (console.start + console.width) % console.size;

    set_frame(console.start);
    if(console.index>=2*console.size) {
	    console.index %= 2*console.size;
    }
}

void print_character(char c, int attribute)
{
    if (c == '\n' || c == '\r') {
	print_newline();
	return;
    }
    const int out = c & 0xff | attribute << 8;
    console.buffer[console.index % console.size] = out;
    console.buffer[console.index % console.size + console.size] = out;

    /*console.buffer[console.ypos * console.width + console.xpos] =
       (c & 0xff) | (attribute << 8); */


    console.index++;
    console.xpos++;

    if (console.xpos >= console.width) {
	print_newline();
    }
    //set_cursor(console.index % console.size);
}

void print_string(char *s)
{
    const int light_gray = 0x07;
    while (*s) {
	print_character(*s++, light_gray);
    }
}


typedef long integer;
void print_int(integer x)
{
    const int radix = 10;
    const int buffer_size = 2 + sizeof(integer) * 3;
    char buffer[buffer_size];
    int start = buffer_size - 1;
    int negative = x < 0;

    buffer[buffer_size - 1] = 0;
    x = negative ? -x : x;
    if (x == 0) {
	buffer[--start] = '0';
    }
    while (x != 0) {
	buffer[--start] = '0' + (x % radix);
	x /= radix;
    }
    if (negative) {
	buffer[--start] = '-';
    }
    print_string(&buffer[start]);
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
}

void fizzbuzz()
{
    for (int i = 1; i < 101; i++) {
	int fallthrough = 1;
	if (i % 3 == 0) {
	    print_string("Fizz");
	    fallthrough = 0;
	}
	if (i % 5 == 0) {
	    print_string("Buzz");
	    fallthrough = 0;
	}
	if (fallthrough) {
	    print_int(i);
	}
	print_string("\n");
    }
}

static inline void outb(unsigned short port, unsigned char val)
{
    asm volatile ("outb %0, %1"::"a" (val), "Nd"(port));
}

static inline void outw(unsigned short port, unsigned short val)
{
    asm volatile ("outw %0, %1"::"a" (val), "Nd"(port));
}

void
set_vga_register(unsigned short port, unsigned char index,
		 unsigned char value)
{
    /*outb(port, index);
       outb(port+1, value); */
    outw(port, ((unsigned short) value << 8) | index);
}

void set_cursor(int address)
{
    // Address low.
    set_vga_register(0x3d4, 0x0f, address & 0xff);
    // Address high.
    set_vga_register(0x3d4, 0x0e, (address >> 8) & 0xff);
}

void set_frame(int address)
{
    // Address high.
    set_vga_register(0x3d4, 0x0c, (address >> 8) & 0xff);
    // Address low.
    set_vga_register(0x3d4, 0x0d, address & 0xff);
}

void debug()
{
    initialize_text_console();
    clear_console();

    for (integer i = 0;; i++) {
	print_string("foo! ");
	print_int(i);
	print_string("\n");
    }

}
