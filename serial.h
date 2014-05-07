/* serial.h */

void serial_enqueue(int port, uint8_t c);
void handle_serial_interrupt(int port);
void enable_serial(int port);
uint8_t read_serial(int port);
void write_serial(int port, uint8_t c);

#define SERIAL_COM1_PORT	0x3f8
#define SERIAL_COM2_PORT	0x2f8
#define SERIAL_COM3_PORT	0x3e8
#define SERIAL_COM4_PORT	0x2e8
