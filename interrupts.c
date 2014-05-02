
#include "util.h"


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct interrupt_descriptor {
    uint32_t offset_0:16;	// bits 0 to 15 of the offset
    uint32_t selector:16;	// code segment selector
     uint32_t:8;

    /* 0x5: 32 bit task gate;
       0x6: 16-bit interrupt gate;
       0x7: 16-bit trap gate
       0xe: 32-bit interrupt gate;
       0xf: 32-bit trap gate
     */
    uint32_t gate_type:4;

    /* zero for interrupt gates. */
    uint32_t storage_segment:1;

    /* descriptor privilege level */
    uint32_t dpl:2;
    /* Can be set to 0 for unused interrupts. */
    uint32_t present:1;

    /* bits 16 to 32 of the offset */
    uint32_t offset_1:16;
};

struct isr {
    uint8_t code[56];
    uint32_t index;
    uint32_t offset;
};


extern struct isr isrs[];
extern struct interrupt_descriptor idt[];


inline static void set_interrupt_descriptor(struct interrupt_descriptor
					    *desc, uint32_t offset,
					    uint16_t selector,
					    uint8_t gate_type,
					    uint8_t storage_segment,
					    uint8_t dpl, uint8_t present)
{
    desc->offset_0 = offset & 0xffff;
    desc->offset_1 = (offset >> 16) & 0xffff;
    desc->selector = selector & 0xffff;
    desc->gate_type = gate_type & 0xf;
    desc->storage_segment = storage_segment & 1;
    desc->dpl = dpl & 0x3;
    desc->present = present & 1;

}

void disable_nmi()
{
    outportb(0x70, inportb(0x70) & 0x7f);

}

void enable_nmi()
{
    outportb(0x70, inportb(0x70) | 0x80);
}

void disable_interrupts()
{
    asm volatile ("cli");
}

void enable_interrupts()
{
    asm volatile ("sti");
}

int rtc_count = 0;
int pit_count = 0;
int pit_last = 0;

#define PIC_MASTER_OFFSET	0x68
#define PIC_SLAVE_OFFSET	(PIC_MASTER_OFFSET+8)

#define PIC_MASTER_ISR(num)	((num)+PIC_MASTER_OFFSET)
#define PIC_SLAVE_ISR(num)	((num)+PIC_SLAVE_OFFSET-8)

#define PIC_MASTER_COMMAND	0x20
#define PIC_SLAVE_COMMAND	0xa0
#define PIC_MASTER_DATA		(PIC_MASTER_COMMAND + 1)
#define PIC_SLAVE_DATA		(PIC_SLAVE_COMMAND + 1)
#define PIC_ICW1_ICW4		(1 << 0)
#define PIC_ICW1_INIT		(1 << 4)

#define PIC_ICW4_8086		(1 << 0)

static inline void io_wait()
{
    /* Use an used port. */
    outportb(0x80, 0);
}


void set_irq_mask(int irq, int x)
{
    int port, index;
    if (irq < 8) {
	port = PIC_MASTER_DATA;
	index = irq;
    } else if (irq < 16) {
	port = PIC_SLAVE_DATA;
	index = irq - 8;
    } else {
	return;
    }
    disable_interrupts();
    int mask = inportb(port);
    int input_bit = (x != 0) << index;
    int input_mask = 1 << index;
    if ((mask ^ input_bit) & input_mask) {
	mask &= ~input_mask;
	mask |= input_bit;
	outportb(port, mask);
    }
    enable_interrupts();
}

void initialize_pic()
{
    disable_interrupts();
    outportb(PIC_MASTER_DATA, 0xff);
    outportb(PIC_SLAVE_DATA, 0xff);
    enable_interrupts();
}

void remap_pic(int master_offset, int slave_offset)
{
    int master_mask = inportb(PIC_MASTER_DATA);
    int slave_mask = inportb(PIC_SLAVE_DATA);
    const int slave_irq = 2;
    const int slave_cascade_irq = 1;


    /* Start initialization sequence. */
    outportb(PIC_MASTER_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();

    outportb(PIC_SLAVE_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();

    /* Set the offsets. */
    outportb(PIC_MASTER_DATA, master_offset);
    io_wait();

    outportb(PIC_SLAVE_DATA, slave_offset);
    io_wait();

    /* Tell the master PIC that there is a slave at IRQ2. */
    outportb(PIC_MASTER_DATA, 1 << slave_irq);
    io_wait();

    /* Tell the slave PIC that its cascade identity is IRQ1 */
    outportb(PIC_SLAVE_DATA, 1 << slave_cascade_irq);
    io_wait();

    outportb(PIC_MASTER_DATA, PIC_ICW4_8086);
    io_wait();
    outportb(PIC_SLAVE_DATA, PIC_ICW4_8086);
    io_wait();

    /* Restore the masks. */
    outportb(PIC_MASTER_DATA, master_mask);
    outportb(PIC_SLAVE_DATA, slave_mask);
}

void handle_irq(int irq)
{
    switch (irq) {
    case 0:
	/* system timer */
	{
	    pit_count++;
#if 0
	    if ((pit_count & 0xff) == 0) {
		print_string("PIT: ");
		print_int(pit_count);
		print_string("\n");
	    }
#endif
	}
	break;
    case 1:
	/* keyboard */
	{
	    /* Get the scan code. */
	    int k = inportb(0x60);



	    int c = inportb(0x61);
	    outportb(0x61, c | 0x80);
	    outportb(0x61, c);

	    if ((k & 0x80) != 0) {
		print_string("key: ");
		print_int(k & 0x7f);
		print_string("; repeat: ");
		print_int((k & 0x80) == 0);
		print_string("\n");
	    }

	}
	break;


    case 8:
	/* real time clock */
	{
	    /* Read RTC register C. */
	    outportb(0x70, 0x0c);
	    int t = inportb(0x71);

	    print_string("rtc ");
	    print_hex(t);
	    print_string("; count ");
	    print_int(++rtc_count);
	    print_string("; PIT ");

	    print_int(pit_count - pit_last);
	    pit_last = pit_count;

	    print_string("\n");
	}
	break;
    case 12:
	/* PS/2 mouse */
	{
	    print_string("mouse\n");
	}
	break;
    }

}

void isr_handler(uint32_t num)
{
    if ((num & -8) == PIC_MASTER_OFFSET) {
	handle_irq(num - PIC_MASTER_OFFSET);
	/* end of interrupt */
	outportb(0x20, 0x20);
	return;
    } else if ((num & -8) == PIC_SLAVE_OFFSET) {
	handle_irq(num - PIC_SLAVE_OFFSET + 8);

	/* end of interrupt */
	outportb(0xa0, 0x20);
	outportb(0x20, 0x20);

	return;
    }

    switch (num) {
    default:
	print_string("interrupt ");
	print_int(num);
	print_string(" was called\n");
	return;
    }
}

#define CODE_SEGMENT 0x08
void populate_interrupts()
{
    /* 32 bit interrupt gate */
    const int gate_type = 0xe;

    disable_interrupts();
    for (int i = 0; i < 256; i++) {
	uint32_t offset = (uint32_t) & isrs[i];
	isrs[i].index = i;
	isrs[i].offset = (uint32_t) & isr_handler;
	set_interrupt_descriptor(&idt[i], offset, CODE_SEGMENT, gate_type,
				 0, 0, 1);
    }
    enable_interrupts();
}

#define call_interrupt(interrupt_number)	asm volatile("int %0" : : "i" (interrupt_number));

void enable_rtc(int rate)
{
    disable_interrupts();

#if 1
    disable_nmi();
    outportb(0x70, 0x8b);	// select register B, and disable NMI
    int reg_b = inportb(0x71);	// read the current value of register B
    outportb(0x70, 0x8b);	// set the index again (a read will reset the index to register D)
    outportb(0x71, reg_b | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B

    outportb(0x70, 0x8a);
    int reg_a = inportb(0x71);
    outportb(0x70, 0x8a);
    outportb(0x71, (reg_a & 0xf0) | (rate & 0x0f));

    /* Make IRQ 8 fire. */
    /*inportb(0xa1);
       outportb(0xa1, 0); */
    set_irq_mask(8, 0);
    enable_nmi();
#endif

    enable_interrupts();
}

#define PIT_FREQUENCY	1193180
#define PIT_CHANNEL(n)	((n)+0x40)
#define PIT_COMMAND	0x43
void enable_pit(int frequency)
{
    union {
	struct {
	    uint32_t bcd:1;
	    uint32_t mode:3;
	    uint32_t access:2;
	    uint32_t channel:2;
	} s;
	uint8_t b;
    } mode_command;

    disable_interrupts();
    int divisor = PIT_FREQUENCY / frequency;

    mode_command.b = 0;
    mode_command.s.channel = 0;
    /* square wave generator */
    mode_command.s.mode = 0x03;
    /* low-byte/high-byte access */
    mode_command.s.access = 0x3;
    mode_command.s.bcd = 0;
    outportb(PIT_COMMAND, mode_command.b);

/* Send the frequency divisor low-byte first. */
    outportb(PIT_CHANNEL(0), divisor & 0xff);
    outportb(PIT_CHANNEL(0), (divisor >> 8) & 0xff);

    enable_interrupts();

    pit_count = 0;
    set_irq_mask(0, 0);
}

void enable_keyboard()
{
    set_irq_mask(1, 0);
}

void show_interrupts()
{
    for (int i = 0; i < 256; i++) {
#if 0
	print_string("interrupt ");
	print_int(i);
	print_string(" : ");
	print_hex(idt[i].offset_1);
	print_string(":");
	print_hex(idt[i].offset_0);
	print_string(" ");
	print_hex(idt[i].selector);
	print_string(" ");
	print_hex(idt[i].gate_type);
	print_string(" ");
	print_hex(idt[i].storage_segment);
	print_string(" ");
	print_hex(idt[i].dpl);
	print_string(" ");
	print_hex(idt[i].present);
#else
	print_hex((uint32_t) isrs[i].offset);
	{
	    unsigned char *foo = (unsigned char *) isrs[i].offset;
	    for (int i = 0; i < 10; i++) {
		print_string(" ");
		print_hex((int) foo[i]);
	    }

	}
#endif
	print_string("\n");

    }
}


void setup_interrupts()
{
    remap_pic(0x68, 0x70);
    initialize_pic();

    set_irq_mask(1, 0);
    set_irq_mask(2, 0);
    set_irq_mask(12, 0);

    //remap_pic(0x08, 0x70);
    //enable_keyboard();
    enable_rtc(15);
    enable_pit(2000);
}
