/* serial.c */

#include "os.h"
#include "util.h"
#include "serial.h"
#include "interrupts.h"


#define SERIAL_BUFFER_SIZE	3

struct serial_buffer {
    int base;
    int count;
    uint8_t data[SERIAL_BUFFER_SIZE];
};
struct serial_buffer serial_out[4];
struct serial_buffer serial_in[4];

static inline int port_to_index(int port)
{
    /* Map [0x3f8,0x2f8,0x3e8,0x2e8] to [0,1,2,3] respectively. */
    return ~(((port >> 4) & 1) | ((port >> (8 - 1)) & 2)) & 3;
}

/* Is the port ready for sending a byte? */
static int is_transmit_empty(int port)
{
    return (inportb(port + 5) & 0x20) != 0;
}

/* Is there a received byte in the serial port? */
static int serial_received(int port)
{
    return inportb(port + 5) & 1;
}

/* Read a byte from the port. */
uint8_t read_serial(int port)
{
    while (!serial_received(port));

    return inportb(port);
}

/* Write a byte to the serial port. */
void write_serial(int port, uint8_t c)
{
    while (!is_transmit_empty(port));

    outportb(port, c);

}



int enqueue_buffer(struct serial_buffer *buffer, uint8_t c)
{
    if (buffer->count >= SERIAL_BUFFER_SIZE) {
	return 0;
    }
    buffer->data[(buffer->base + buffer->count) % SERIAL_BUFFER_SIZE] = c;
    buffer->count++;
    if (buffer->count >= SERIAL_BUFFER_SIZE) {
	buffer->count %= SERIAL_BUFFER_SIZE;
    }
    return 1;
}

uint8_t dequeue_buffer(struct serial_buffer * buffer)
{
    uint8_t c = buffer->data[buffer->base % SERIAL_BUFFER_SIZE];
    buffer->count--;
    buffer->base++;
    if (buffer->base >= SERIAL_BUFFER_SIZE) {
	buffer->base %= SERIAL_BUFFER_SIZE;
    }
    return c;
}


void serial_enqueue(int port, uint8_t c)
{
    struct serial_buffer *buffer = &serial_out[port_to_index(port)];

    /* Try to enqueue. If the buffer is full then wait until the buffer has space. */
    while (!enqueue_buffer(buffer, c)) {
	io_wait();
    }
    /* Prime the transmitter. */
    if (buffer->count > 0 && is_transmit_empty(port)) {
	outportb(port, dequeue_buffer(buffer));
    }
}

char last_line[1024];
int last_line_index = 0;
void consume_buffer(struct serial_buffer * buffer) {
	while(buffer->count > 0) {
		int c = dequeue_buffer(buffer);

			last_line[last_line_index++] = c;
		if(c == '\n' || c ==  '\r') {
			last_line[last_line_index] = '\0';
			//print_string(last_line);
			last_line[0] = '\0';
			last_line_index = 0;
		}
	}
}


/* Initialize the serial port. */
void enable_serial(int port)
{
    /* Disable all interrupts. */
    outportb(port + 1, 0x00);

    /* Enable Divide Latch Access Bit. */
    outportb(port + 3, 0x80);

    /* Set the divisor to 3 (38400 baud). */
    outportb(port + 0, 0x03);
    outportb(port + 1, 0x00);

    /* 8 bits, no parity, one stop bit */
    outportb(port + 3, 0x03);
    /* Enable FIFO, clear them, with 14-byte threshold. */
    outportb(port + 2, 0xc7);

    /* Enable IRQs and set RTS/DSR. */
    outportb(port + 4, 0x0b);

    /* Enable byte reception interrupt. */
    outportb(port + 1, inportb(port + 1) | 0x01);

    /* Enable transmitter empty interrupt. */
    outportb(port + 1, inportb(port + 1) | 0x02);


    /* Enable the port in the PIC. */
    if(port & 0x100) {
    outportb(0x21, inportb(0x21) & 0xef);
	} else {
    outportb(0x21, inportb(0x21) & 0xf7);
	}
    /* Set up the queues. */
    struct serial_buffer *out_buffer = &serial_out[port_to_index(port)];
    out_buffer->count = 0;
    out_buffer->base = 0;

    last_line[0] = '\0';
    struct serial_buffer *in_buffer = &serial_in[port_to_index(port)];
    in_buffer->count = 0;
    in_buffer->base = 0;
}

/* TODO: Handle COM3 and COM4 */
void handle_serial_interrupt(int port)
{
    uint8_t iir = inportb(port + 2);

    switch ((iir >> 1) & 0x03) {
    case 0x01:
	/* transmitter empty */
	{
	    struct serial_buffer *buffer =
		&serial_out[port_to_index(port)];

	    while (buffer->count > 0 && is_transmit_empty(port)) {
		outportb(port, dequeue_buffer(buffer));
	    }
	}
	break;
    case 0x02:
	/* data available */
	{
		struct serial_buffer * buffer =
			&serial_in[port_to_index(port)];

		//print_string("serial in\n");
		while(serial_received(port)) {
		int c = inportb(port);
		enqueue_buffer(buffer, c);
		consume_buffer(buffer);
		}
	}
	break;
    }

}
