/* interrupts.c */

#include "util.h"
#include "interrupts.h"
#include "serial.h"


int rtc_count = 0;
int pit_count = 0;
int pit_last = 0;

/* interrupt descriptor table entry */
struct interrupt_descriptor {
    uint32_t offset_0:16;	// bits 0 to 15 of the offset
    uint32_t selector:16;	// code segment selector
     uint32_t:8;

    /* 0x5: 32 bit task gate;
       0xe: 32-bit interrupt gate;
       0xf: 32-bit trap gate
       0x6: 16-bit interrupt gate;
       0x7: 16-bit trap gate
     */
    uint32_t type:3;
    uint32_t size:1;

    /* zero for interrupt gates. */
    uint32_t storage_segment:1;

    /* descriptor privilege level */
    uint32_t dpl:2;
    /* Can be set to 0 for unused interrupts. */
    uint32_t present:1;

    /* bits 16 to 32 of the offset */
    uint32_t offset_1:16;
} __attribute__ ((packed));

struct isr {
    uint8_t code[ISR_SIZE - 3 * 4];
    /* 4 if there is an error code. */
    uint32_t error_code_present;
    uint32_t vector;
    uint32_t offset;
} __attribute__ ((packed));


extern struct isr isrs[];
extern struct interrupt_descriptor idt[];

enum gate_type {
    task = 0x05,
    interrupt = 0x06,
    trap = 0x07,
};

enum interrupt_vector {
    divide_error = 0,
    overflow = 4,
    double_fault = 8,
    invalid_tss = 10,
    segment_absent = 11,
    stack_fault = 12,
    protection_fault = 13,
    page_fault = 14
};

inline static void set_interrupt_descriptor(struct interrupt_descriptor
					    *desc, uint32_t offset,
					    uint16_t selector,
					    enum gate_type type,
					    int size,
					    uint8_t dpl, uint8_t present)
{
#if 1
    const struct interrupt_descriptor entry = {
	.offset_0 = offset & 0xffff,
	.offset_1 = (offset >> 16) & 0xffff,
	.selector = selector,
	.type = type,
	.size = type != task && size == 32,
	.storage_segment = 0,
	.dpl = dpl,
	.present = 1
    };

    *desc = entry;
    /*desc->dpl = dpl;
       desc->present = 1;
       desc->storage_segment = 0;

       desc->offset_0 = offset & 0xffff;
       desc->offset_1 = (offset >> 16) & 0xffff;
       desc->selector = selector & 0xffff;
       desc->type = type ;
       desc->size = (type == task || size != 16); */

#else
    desc->offset_0 = offset & 0xffff;
    desc->offset_1 = (offset >> 16) & 0xffff;
    desc->selector = selector & 0xffff;
    desc->type = type;
    desc->size = 1;
    desc->storage_segment = 0;
    desc->dpl = dpl & 0x3;
    desc->present = 1;
#endif
}

/* Enable NMIs if state is 1, disable NMIs if state is 0. */
static inline void set_nmi(int state)
{
    outportb(0x70, (inportb(0x70) & 0x7f) | (state << 7));
}

static inline void disable_interrupts()
{
    asm volatile ("cli");
}

static inline void enable_interrupts()
{
    asm volatile ("sti");
}

/* Set the IRQ mask bit. */
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

/* Initialize the PIC and disable all requests. */
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
    case IRQ_TIMER:
	/* system timer */
	{
	    pit_count++;
#if 1
	    if ((pit_count & 0x3ff) == 0) {
		update();
	    }
#endif
	}
	break;

    case IRQ_KEYBOARD:
	/* keyboard */
	{
	    /* Get the scan code. */



#if 1
	    int k = inportb(0x60);
	    if ((k & 0x80) != 0) {
		print_string("key: ");
		print_int(k & 0x7f);
		print_string("; repeat: ");
		print_int((k & 0x80) == 0);
		print_string("\n");
	    }
#endif
	    int c = inportb(0x61);
	    outportb(0x61, c | 0x80);
	    outportb(0x61, c);

	}
	break;

    case IRQ_RTC:
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
    case IRQ_MOUSE:
	/* PS/2 mouse */
	{
	    print_string("mouse\n");
	}
	break;


    case IRQ_SERIAL_PORT_1:
	{
	    handle_serial_interrupt(SERIAL_COM1_PORT);
	    print_string("serial 1\n");
	}
	break;
    case IRQ_SERIAL_PORT_2:
	{
	    handle_serial_interrupt(SERIAL_COM2_PORT);
	    print_string("serial 2\n");
	}
	break;

    }

}

void isr_handler(const enum interrupt_vector num, uint32_t error_code)
{
    switch (num) {
    case segment_absent:
	{
	    print_string("segment not present\n");
	}
	break;

    case protection_fault:
	{
	    print_string("general protection fault\n");
	}
	break;

    case page_fault:
	{
	    print_string("page fault\n");
	}
	break;


    default:
	if ((num & -8) == PIC_MASTER_OFFSET) {
	    handle_irq(num - PIC_MASTER_OFFSET);

	    /* end of interrupt */
	    outportb(0x20, 0x20);
	    break;
	} else if ((num & -8) == PIC_SLAVE_OFFSET) {
	    handle_irq(num - PIC_SLAVE_OFFSET + 8);

	    /* end of interrupt */
	    outportb(PIC_SLAVE_COMMAND, PIC_END_OF_INTERRUPT);
	    outportb(PIC_MASTER_COMMAND, PIC_END_OF_INTERRUPT);
	    break;
	}

	print_string("interrupt ");
	print_int(num);
	print_string(" was called\n");

	break;
    }
}

void populate_interrupts()
{
    disable_interrupts();

    for (int i = 0; i < 256; i++) {
	uint32_t offset = (uint32_t) & isrs[i];
	isrs[i].vector = i;
	isrs[i].offset = (uint32_t) & isr_handler;
	set_interrupt_descriptor(&idt[i], offset, CODE_SEGMENT, interrupt,
				 32, 0, 1);
    }
    const int error_code_interrupts[] =
	{ double_fault, invalid_tss, segment_absent, stack_fault,
	protection_fault, page_fault
    };

    for (int i = 0; i < sizeof(error_code_interrupts) / sizeof(int); i++) {
	isrs[i].error_code_present = 1;
    }

    enable_interrupts();
}

void enable_rtc(int rate)
{
    disable_interrupts();

    set_nmi(0);
    outportb(0x70, 0x8b);	// select register B, and disable NMI
    int reg_b = inportb(0x71);	// read the current value of register B
    outportb(0x70, 0x8b);	// set the index again (a read will reset the index to register D)
    outportb(0x71, reg_b | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B

    outportb(0x70, 0x8a);
    int reg_a = inportb(0x71);
    outportb(0x70, 0x8a);
    outportb(0x71, (reg_a & 0xf0) | (rate & 0x0f));

    /* Make IRQ 8 fire. */
    set_irq_mask(8, 0);
    set_nmi(1);

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
    set_irq_mask(IRQ_KEYBOARD, 0);
}

void setup_interrupts()
{
    remap_pic(IRQ_ISR_BASE, IRQ_ISR_BASE + 8);
    initialize_pic();

    set_irq_mask(2, 0);
    set_irq_mask(12, 0);

    enable_keyboard();
    //enable_rtc(15);
    //enable_pit(2000);
}
