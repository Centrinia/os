

#define ISR_SIZE				64
#define call_interrupt(interrupt_number)	asm volatile("int %0" : : "i" (interrupt_number));

#if !defined(__ASSEMBLER__)
void setup_interrupts();
void enable_rtc(int rate);
#endif

