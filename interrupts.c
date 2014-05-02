
#include "util.h"


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct interrupt_descriptor {
	uint32_t offset_0 : 16; // bits 0 to 15 of the offset
	uint32_t selector : 16; // code segment selector
	uint32_t : 8; 
	
	/* 0x5: 32 bit task gate;
	   0x6: 16-bit interrupt gate;
	   0x7: 16-bit trap gate
	   0xe: 32-bit interrupt gate;
           0xf: 32-bit trap gate
	*/
	uint32_t gate_type : 4; 

	/* zero for interrupt gates. */
	uint32_t storage_segment : 1;

	/* descriptor privilege level */
	uint32_t dpl : 2;
	/* Can be set to 0 for unused interrupts. */
	uint32_t present : 1;

	/* bits 16 to 32 of the offset */
	uint32_t offset_1 : 16;
};

struct isr {
	uint8_t code[56];
	uint32_t index;
	void* offset;
};

extern struct isr isrs[];
extern struct interrupt_descriptor idt[];


inline static void set_interrupt_descriptor(struct interrupt_descriptor * desc, uint32_t offset, uint16_t selector, uint8_t gate_type, uint8_t storage_segment, uint8_t dpl, uint8_t present) {
	desc->offset_0 = offset & 0xffff;
	desc->offset_1 = (offset>>16) & 0xffff;
	desc->selector = selector & 0xffff;
	desc->gate_type = gate_type & 0xf;
	desc->storage_segment = storage_segment & 1;
	desc->dpl = dpl & 0x3;
	desc->present = present & 1;

}

void isr_handler(uint32_t num) {
	switch(num) {
		case 0x09:
			{
				/* Get the scan code. */
				int k = inportb(0x60);



				int c = inportb(0x61);
				outportb(0x61, c|0x80);
				outportb(0x61, c);


				/* End the interrupt. */
			outportb(0x20,0x20);


			if((k&0x80) != 0) {
				print_string("key: ");
				print_int(k&0x7f);
				print_string("; repeat: ");
				print_int((k&0x80) == 0);
				print_string("\n");
			}

			}
			break;
		default:
	print_string("interrupt ");
	print_int(num);
	print_string(" was called\n");
			break;
	}
}

#define CODE_SEGMENT 0x08
void populate_interrupts() {
	/* 32 bit interrupt gate */
	const int gate_type  = 0xe;

	for(int i=0;i<256;i++) {
		uint32_t offset = &isrs[i];
		isrs[i].index = i;
		isrs[i].offset = isr_handler;
		set_interrupt_descriptor(&idt[i], offset,CODE_SEGMENT, gate_type, 0, 0,1);
	}
}

void disable_interrupts() {
	asm volatile("cli");
}

void enable_interrupts() {
	asm volatile ("sti");
}

void enable_rtc() {
	disable_interrupts();

	outportb(0x70, 0x8B);		// select register B, and disable NMI
	char prev=inportb(0x71);	// read the current value of register B
	outportb(0x70, 0x8B);		// set the index again (a read will reset the index to register D)
	outportb(0x71, prev | 0x40);	// write the previous value ORed with 0x40. This turns on bit 6 of register B

	enable_interrupts();

}

void enable_keyboard() {
	disable_interrupts();
	outportb(0x21,0xfd);
	outportb(0xa1,0xff);
	enable_interrupts();
}
void show_interrupts() {
	for(int i=0;i<256;i++) {
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
		print_hex(isrs[i].offset);
		{
			unsigned char * foo = isrs[i].offset;
			for(int i=0;i<10;i++) {
			print_string(" ");
			print_hex((int)foo[i]);
			}
		}
#endif
		print_string("\n");
	
	}
}



