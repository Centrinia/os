/* main.c */

#include "util.h"
#include "serial.h"

void update()
{
    //vga_update();

}

struct e820 {
    uint32_t base_address[2];
    uint32_t length[2];
    uint32_t type;
     uint32_t:32;
};

void print_e820()
{
    int e820_entries = *((int *) (512 * 1024));
    struct e820 *entries = (struct e820 *) (512 * 1024 + 4);
    print_int(e820_entries);
    print_string("\n");

    for (int i = 0; i < e820_entries; i++) {
	print_int(i);
	print_string(" :: ");
	print_hex_padded(entries[i].base_address[1], 8);
	print_string(":");
	print_hex_padded(entries[i].base_address[0], 8);
	print_string(" ");

#if 1
	print_hex_padded(entries[i].length[1], 8);
	print_string(":");
	print_hex_padded(entries[i].length[0], 8);
#else
	print_int(entries[i].length[0]);
#endif

	print_string(" ");

	print_hex(entries[i].type);
	print_string("\n");

    }

}

int main()
{

    initialize_text_console();

    print_string("Hello World!\n");
    setup_interrupts();
    setup_paging();
    //enable_serial(SERIAL_COM1_PORT);

    vga_main();
    for(;;) {
	    vga_update();
    }
    //enable_keyboard();
    //enable_rtc(10);
    //show_interrupts();



    print_e820();
    print_hex_padded(*((uint32_t *) 0xffff0) & 0xffffff, 8);
    print_string("\n");



#if 0
    for (int i = 0;; i++) {
	print_int(i);
	print_string("\n");
    }
#endif

#if 0
    char buffer[1024];

    int index = 0;
    for (;;) {
	int c = read_serial(SERIAL_COM1_PORT);
	if (index == 1023 || c == '\n' || c == '\n' || c == '\0') {
	    buffer[index - 1] = '\0';
	    print_string("received: \"");
	    print_string(buffer);
	    print_string("\"\n");
	    index = 0;
	} else {
	    buffer[index++] = c;
	}
    }
    //asm volatile("int $0x69");
#endif

#if 1
    uint32_t out[4];
    cpuid(out, 0x01);

#if 1
    for (int i = 0; i < 4; i++) {
	print_hex(out[i]);
	print_string(" ");
    }
    print_string("\n");
#else
    print_int((out[3] >> 3) & 1);
    print_string("\n");
#endif
    uint32_t *buf3 = 0x4079d4;
    //uint32_t *buf3 = 0x400004;
    print_hex(buf3[0]);
    print_string("\n");
    buf3[0]++;
    print_hex(buf3[0]);
    print_string("\n");

    cpuid(out, 0x01);
    //print_string("finished\n");
#endif

#if 0
    const int size = 12 << 20;
    //const int size = 0x1e75;
    uint32_t *buf2 = (uint32_t *) (1 << 20);
    uint32_t *buf = (uint32_t *) ((4UL << 20) - 16);
    for (int i = 0; i < size; i++) {
	uint32_t x = rand();
	buf[i] = x;
	buf2[i] = x;
	if (buf[i] != x) {
#if 1
	    print_int(i * 4);
	    print_string(" : ");
	    print_int(x);
	    print_string("\n");
#endif
	}
#if 0
	else {

	    print_hex(i * 4 + (uint32_t) buf);
	    print_string(" succeeded\n");
	}
#endif

    }

#if 1
    int failure = 0;
    for (int i = 0; i < size; i++) {
	if (buf[i] != buf[i]) {
	    print_int(i * 4);
	    print_string(" : ");
	    print_hex(buf[i]);
	    print_string(" ");
	    print_hex(buf2[i]);
	    print_string("\n");
	    failure = 1;
	}
    }
    if (!failure) {
	print_string("success\n");

    }
#endif

#endif

}
