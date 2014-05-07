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

static inline void io_wait()
{
    /* Use an used port. */
    outportb(0x80, 0);
}




void
set_vga_register(unsigned short port, unsigned char index,
		 unsigned char value);

void print_int(int x);
void print_hex(unsigned int x);
void print_string(char *s);
void print_hex_padded(unsigned int x, int width);

unsigned int rand();
void setup_paging();


void initialize_text_console();
void setup_interrupts();
void update();
int main();

void cpuid(uint32_t * out, uint32_t index);
