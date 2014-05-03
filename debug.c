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
    if (!(registerB & 0x04)) {
	// TODO: Do the BCD conversion.
    }
#endif
    if (century == 0) {
	date->year += (CURRENT_YEAR / 100) * 100;
	if (date->year < CURRENT_YEAR) {
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

    if (a->month > 1 && ((a->year % 4 == 0)
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


unsigned int random_seed = 0;
unsigned int rand()
{
    random_seed = random_seed * 1664525UL + 1013904223UL;
    return random_seed;
}

void debug()
{


    //for (;;) 
    {
#if 0
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
