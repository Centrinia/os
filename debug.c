/* debug.c */

#include "util.h"

struct date {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    int year;
};

#define CMOS_ADDRESS_PORT 0x70
#define CMOS_DATA_PORT 0x71
#define CMOS_STATUS_REGISTER_A 0x0a
int rtc_read(int reg)
{
    outportb(CMOS_ADDRESS_PORT, reg);
    return inportb(CMOS_DATA_PORT);
}

int rtc_update_in_progress()
{
    // Return bit 7 of status register A.
    return rtc_read(CMOS_STATUS_REGISTER_A | 0x80) & 0x80;
}

#define CURRENT_YEAR 2014
void poll_date(struct date *date)
{
    while (rtc_update_in_progress());
    date->second = rtc_read(0x00);
    date->minute = rtc_read(0x02);
    date->hour = rtc_read(0x04);
    date->day = rtc_read(0x07);
    date->month = rtc_read(0x08);

    date->year = rtc_read(0x09);
    int century = rtc_read(0x32);

#if 0
    int registerB = rtc_read(CMOS_STATUS_REGISTER_B);
    if(!(registerB & 0x04)) {
	    // TODO: Do the BCD conversion.
    }
#endif
    if (century == 0) {
      date->year += (CURRENT_YEAR/100)*100;
       if(date->year < CURRENT_YEAR) {
	       date->year = CURRENT_YEAR;
       }
    } else {
	    date->year += century * 100;
    }
}

//const static int month_days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
const static int cumulative_month_days[] =
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

int seconds_since(const struct date *a)
{
    int result = 0;
    result = a->year * 365;
    result += a->year / 4;
    result -= a->year / 100;
    result += a->year / 400;
    result += cumulative_month_days[a->month];

    if (a->month > 1
	&& ((a->year % 4 == 0)
	    && (a->year % 100 != 0 || a->year % 400 == 0))) {
	// Add leap day if it is after Feburary on a leap year.
	result += 1;
    }

    result += a->day;
    // result is the number of days since the beginning of 1/1/0 by now.

    result *= 24;
    result += a->hour;
    result *= 60;
    result += a->minute;
    result *= 60;
    result += a->second;
    return result;
}

void read_date(struct date *date)
{
    struct date last;

    poll_date(date);
    do {
	last = *date;
	poll_date(date);
    } while (seconds_since(date) != seconds_since(&last));
}


// From http://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
#define	VGA_AC_INDEX		0x3C0
#define	VGA_AC_WRITE		0x3C0
#define	VGA_AC_READ		0x3C1
#define	VGA_MISC_WRITE		0x3C2
#define VGA_SEQ_INDEX		0x3C4
#define VGA_SEQ_DATA		0x3C5
#define	VGA_DAC_READ_INDEX	0x3C7
#define	VGA_DAC_WRITE_INDEX	0x3C8
#define	VGA_DAC_DATA		0x3C9
#define	VGA_MISC_READ		0x3CC
#define VGA_GC_INDEX 		0x3CE
#define VGA_GC_DATA 		0x3CF
/*			COLOR emulation		MONO emulation */
#define VGA_CRTC_INDEX		0x3D4	/* 0x3B4 */
#define VGA_CRTC_DATA		0x3D5	/* 0x3B5 */
#define	VGA_INSTAT_READ		0x3DA


#define	VGA_NUM_SEQ_REGS	5
#define	VGA_NUM_CRTC_REGS	25
#define	VGA_NUM_GC_REGS		9
#define	VGA_NUM_AC_REGS		21
#define	VGA_NUM_REGS		(1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
						VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

unsigned char g_80x25_text[] = {
    /* MISC */
    0x67,
    /* SEQ */
    0x03, 0x00, 0x03, 0x00, 0x02,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
    0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00
};


unsigned char g_320x200x256[] = {
    /* MISC */
    0x63,
    /* SEQ */
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};

unsigned char g_320x200x256_modex[] = {
    /* MISC */
    0x63,
    /* SEQ */
    0x03, 0x01, 0x0F, 0x00, 0x06,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};

void write_regs(unsigned char *regs)
{
    unsigned i;

    /* write MISCELLANEOUS reg */
    outportb(VGA_MISC_WRITE, *regs);
    regs++;
    /* write SEQUENCER regs */
    for (i = 0; i < VGA_NUM_SEQ_REGS; i++) {
	outportb(VGA_SEQ_INDEX, i);
	outportb(VGA_SEQ_DATA, *regs);
	regs++;
    }
    /* unlock CRTC registers */
    outportb(VGA_CRTC_INDEX, 0x03);
    outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) | 0x80);
    outportb(VGA_CRTC_INDEX, 0x11);
    outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) & ~0x80);
    /* make sure they remain unlocked */
    regs[0x03] |= 0x80;
    regs[0x11] &= ~0x80;
    /* write CRTC regs */
    for (i = 0; i < VGA_NUM_CRTC_REGS; i++) {
	outportb(VGA_CRTC_INDEX, i);
	outportb(VGA_CRTC_DATA, *regs);
	regs++;
    }
    /* write GRAPHICS CONTROLLER regs */
    for (i = 0; i < VGA_NUM_GC_REGS; i++) {
	outportb(VGA_GC_INDEX, i);
	outportb(VGA_GC_DATA, *regs);
	regs++;
    }
    /* write ATTRIBUTE CONTROLLER regs */
    for (i = 0; i < VGA_NUM_AC_REGS; i++) {
	(void) inportb(VGA_INSTAT_READ);
	outportb(VGA_AC_INDEX, i);
	outportb(VGA_AC_WRITE, *regs);
	regs++;
    }
    /* lock 16-color palette and unblank display */
    (void) inportb(VGA_INSTAT_READ);
    outportb(VGA_AC_INDEX, 0x20);
}
unsigned int random_seed = 0;
unsigned int rand()
{
    random_seed = random_seed * 1664525UL + 1013904223UL;
    return random_seed;
}

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
static inline void set_plane_mask(int mask)
{
    set_vga_register(0x3c4, 0x02, mask);
}

void clear_screen(int color)
{
    volatile unsigned char *buffer = (unsigned char *) 0xa0000;
    set_plane_mask(0x0f);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
	buffer[i] = color;
    }
}

void draw_rectangle(int x0, int y0, int x1, int y1, int color)
{

    volatile unsigned char *buffer = (unsigned char *) 0xa0000;
    if (y0 < 0) {
	y0 = 0;
    }
    if (y1 >= SCREEN_HEIGHT) {
	y1 = SCREEN_HEIGHT - 1;
    }
    for (int plane = 0; plane < 4; plane++) {
	set_plane_mask(1 << plane);
	for (int x = (x0 & -4) + plane; x <= x1; x += 4) {
	    if (x < 0) {
		continue;
	    }
	    if (x >= SCREEN_WIDTH) {
		continue;
	    }
	    const int x_index = (x & 0x3) << 1;

	    for (int y = y0; y <= y1; y++) {
		buffer[(y * SCREEN_WIDTH + x) >> 2] = color;

	    }
	}
    }
}

void draw_trapezoid(int x0, int y00, int y01, int x1, int y10, int y11,
		    int color)
{
    volatile unsigned char *buffer = (unsigned char *) 0xa0000;
    for (int plane = 0; plane < 4; plane++) {
	set_plane_mask(1 << plane);
	for (int x = (x0 & -4) + plane; x <= x1; x += 4) {
	    if (x < 0) {
		continue;
	    }
	    if (x >= SCREEN_WIDTH) {
		continue;
	    }
	    // y = a*(1-t) + b*t = a + t*(b-a)
	    int y0 = y00 + ((y10 - y00) * (x - x0)) / (x1 - x0);
	    int y1 = y01 + ((y11 - y01) * (x - x0)) / (x1 - x0);
	    if (y0 < 0) {
		y0 = 0;
	    }
	    if (y1 >= SCREEN_HEIGHT) {
		y1 = SCREEN_HEIGHT - 1;
	    }
	    int offset = (y0 * SCREEN_WIDTH + x) >> 2;
	    for (int y = y0; y <= y1; y++) {
		//buffer[(y * SCREEN_WIDTH + x) >> 2] = color;
		buffer[offset] = color;
		offset += SCREEN_WIDTH >> 2;
	    }
	}
    }
}

void set_color(int color, int red, int blue, int green)
{
    outportb(0x3C8, color);
    outportb(0x3C9, red);
    outportb(0x3C9, blue);
    outportb(0x3C9, green);
}

void debug()
{


    volatile unsigned char *buffer = (unsigned char *) 0xa0000;

    write_regs(g_320x200x256_modex);
    //for (;;) 
    {
#if 1
//write_regs(g_320x200x256);
	for (int i = 1; i < 256; i++) {
	    unsigned int color = rand();
	    set_color(i, color & 0x3f, (color >> 8) & 0x3f,
		      (color >> 16) & 0x3f);
	}
	clear_screen(0);

	//draw_trapezoid(5,10,70,321,20,40,2);
	//draw_trapezoid(0, 0, 100, 100, 0, 100, 2);
	//draw_rectangle(0, 0, 200, 150,2);
	//draw_rectangle(25, 50, 200, 150,3);
	//draw_trapezoid(0, 0, 100, 100, 0, 100, 2);
	//draw_trapezoid(0, 0, 200, 300, 25, 150, 2);
	//draw_trapezoid(0, 150, 150, 30, 25, 120, 3);
	//draw_trapezoid(0, 0, 0, 30, 25, 150, 4);

#if 1
	const int trapezoids = 1 + rand() % 20;
	for (int i = 0; i < trapezoids; i++)
	    //for(;;) 
	{
	    int x0 = rand() % SCREEN_WIDTH;
	    int x1 = rand() % SCREEN_WIDTH;
	    if (x0 > x1) {
		x1 ^= x0;
		x0 ^= x1;
		x1 ^= x0;
	    }
	    int y00 = (rand() % (SCREEN_HEIGHT)) - SCREEN_HEIGHT * 0;
	    int y01 = (rand() % (SCREEN_HEIGHT)) - SCREEN_HEIGHT * 0;
	    if (y00 > y01) {
		y01 ^= y00;
		y00 ^= y01;
		y01 ^= y00;
	    }

	    int y10 = (rand() % (SCREEN_HEIGHT)) - SCREEN_HEIGHT * 0;
	    int y11 = (rand() % (SCREEN_HEIGHT)) - SCREEN_HEIGHT * 0;

	    if (y10 > y11) {
		y11 ^= y10;
		y10 ^= y11;
		y11 ^= y10;
	    }

	    const int color = (rand() * 0x01010101) >> 24;


	    draw_trapezoid(x0, y00, y01, x1, y10, y11, color);
	}
#endif

#if 0
	for (int plane = 0; plane < 4; plane++) {
	    set_plane_mask(1 << plane);
	    for (int i = 0; i < 320 * 200; i++) {
		buffer[i] = (rand() * 0x01010101) >> 24;
	    }
	}
#endif
#endif
	/*for (int i = 0; i < 320*200; i++) {
	   buffer[i] = 0;
	   } */

#if 0
	write_regs(g_80x25_text);
#if 0
	fizzbuzz();
#else
	for (integer i = 0; i < 10000; i++) {
	    print_string("foo! ");
	    print_int(i);
	    print_string("\n");
	}
#endif
#endif
    }
}
