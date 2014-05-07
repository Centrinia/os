/* string.c */

#include "os.h"
#include "util.h"
#include "serial.h"

void print_string(char *s)
{
	for(int i=0;s[i];i++) {
	  int c = s[i];	
#if USE_TEXT_TERMINAL
	const int light_gray = 0x07;
	show_character(c, light_gray);
#endif
#if USE_SERIAL_CONSOLE
	/*if (c == '\n') {
	    serial_enqueue(SERIAL_COM1_PORT, '\r');
	}*/
	serial_enqueue(SERIAL_COM1_PORT, c);
#endif
    }
}

int sprint_base(char *buffer, int buffer_size, unsigned int x, int radix)
{
    //const int buffer_size = sizeof(unsigned int) * 8;
    //char buffer[buffer_size];
    int start = buffer_size - 1;

    buffer[buffer_size - 1] = '\0';
    if (x == 0) {
	buffer[--start] = '0';
    }
    while (x != 0) {
	int d = x % radix;
	int c = d + (d < 10 ? '0' : 'a' - 10);
	buffer[--start] = c;
	x /= radix;
    }
    //print_string(&buffer[start]);
    return buffer_size - start - 1;
}

void print_base_padded(unsigned int x, int radix, int width)
{
    const int buffer_size =
	(sizeof(unsigned int) * 8 <
	 width ? sizeof(unsigned int) * 8 : width) + 1;
    char buffer[buffer_size];
    int digits = sprint_base(buffer, buffer_size, x, radix);
    while (digits < width) {
	digits++;
	buffer[buffer_size - 1 - digits] = '0';
    }

    print_string(&buffer[buffer_size - 1 - digits]);
}

void print_base(unsigned int x, int radix)
{
    const int buffer_size = sizeof(unsigned int) * 8 + 1;
    char buffer[buffer_size];
    int digits = sprint_base(buffer, buffer_size, x, radix);
    print_string(&buffer[buffer_size - 1 - digits]);
}

void print_signed_base(int x, int radix)
{
    if (x < 0) {
	print_string("-");
	print_base(-x, radix);
    } else {
	print_base(x, radix);
    }
}

void print_int(int x)
{
    print_signed_base(x, 10);
}

void print_hex(unsigned int x)
{
    print_base(x, 16);
}

void print_hex_padded(unsigned int x, int width)
{
    print_base_padded(x, 16, width);
    //print_base(x, 16);
}
