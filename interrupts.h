
#define PIC_END_OF_INTERRUPT	0x20
#define CODE_SEGMENT		0x08

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


#define IRQ_TIMER		0
#define IRQ_KEYBOARD		1
#define IRQ_SERIAL_PORT_2	3
#define IRQ_SERIAL_PORT_1	4
#define IRQ_RTC			8
#define IRQ_MOUSE		12


#define IRQ_ISR_BASE				0x68
#define ISR_SIZE				64
#define call_interrupt(interrupt_number)	asm volatile("int %0" : : "i" (interrupt_number));

#if !defined(__ASSEMBLER__)
void setup_interrupts();
void enable_rtc(int rate);
#endif
