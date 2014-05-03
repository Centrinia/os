/* util.h */


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
static inline void outportb(unsigned short port, unsigned char val)
{
    asm volatile ("outb %0, %1"::"a" (val), "Nd"(port));
}

static inline unsigned char inportb(unsigned short port)
{
    unsigned char val;
    asm volatile ("inb %1, %0":"=a" (val):"Nd"(port));
    return val;
}

static inline void outportw(unsigned short port, unsigned short val)
{
    asm volatile ("outw %0, %1"::"a" (val), "Nd"(port));
}
void
set_vga_register(unsigned short port, unsigned char index,
		 unsigned char value);

typedef long integer;
void print_int(integer x);
void print_hex(integer x);
void print_string(char *s);

unsigned int rand();
void setup_paging();
