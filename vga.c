/* vga.c */

#include "util.h"
struct {
    int start;
    int size;
    int width;
    int height;
    uint8_t *buffer;
    volatile uint8_t *front_buffer;
    volatile uint8_t *back_buffer;
} vga_screen;

void set_frame(int address)
{
    // Address high.
    set_vga_register(0x3d4, 0x0c, (address >> 8) & 0xff);
    // Address low.
    set_vga_register(0x3d4, 0x0d, address & 0xff);
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


void
set_vga_register(unsigned short port, unsigned char index,
		 unsigned char value)
{
    /*outb(port, index);
       outb(port+1, value); */
    outportw(port, ((unsigned short) value << 8) | index);
}

static inline void set_plane_mask(int mask)
{
    set_vga_register(0x3c4, 0x02, mask);
}

void clear_screen(int color)
{
    set_plane_mask(0x0f);
    for (int i = 0; i < vga_screen.width * vga_screen.height; i++) {
	vga_screen.buffer[i] = color;
    }
}

void draw_rectangle(int x0, int y0, int x1, int y1, int color)
{

    if (y0 < 0) {
	y0 = 0;
    }
    if (y1 >= vga_screen.height) {
	y1 = vga_screen.height - 1;
    }
    for (int plane = 0; plane < 4; plane++) {
	set_plane_mask(1 << plane);
	for (int x = (x0 & -4) + plane; x <= x1; x += 4) {
	    if (x < 0) {
		continue;
	    }
	    if (x >= vga_screen.width) {
		continue;
	    }

	    for (int y = y0; y <= y1; y++) {
		vga_screen.buffer[(y * vga_screen.width + x) >> 2] = color;
	    }
	}
    }
}


void draw_trapezoid(int x0, int y00, int y01, int x1, int y10, int y11,
		    int color)
{
#if 0
    for (int plane = 0; plane < 4; plane++) {
	set_plane_mask(1 << plane);
	for (int x = (x0 & -4) + plane; x <= x1; x += 4) {
	    if (x < 0) {
		continue;
	    }
	    if (x >= vga_screen.width) {
		continue;
	    }
	    // y = a*(1-t) + b*t = a + t*(b-a)
	    int y0 = y00 + ((y10 - y00) * (x - x0)) / (x1 - x0);
	    int y1 = y01 + ((y11 - y01) * (x - x0)) / (x1 - x0);
	    if (y0 < 0) {
		y0 = 0;
	    }
	    if (y1 >= vga_screen.height) {
		y1 = vga_screen.height - 1;
	    }
	    int offset = (y0 * vga_screen.width + x) >> 2;
	    for (int y = y0; y <= y1; y++) {
		vga_screen.buffer[offset] = color;
		offset += vga_screen.width >> 2;
	    }
	}
    }
#else
    for (int x = x0; x <= x1; x++) {
	if (x < 0) {
	    continue;
	}
	if (x >= vga_screen.width) {
	    continue;
	}
	// y = a*(1-t) + b*t = a + t*(b-a)
	int y0 = y00 + ((y10 - y00) * (x - x0)) / (x1 - x0);
	int y1 = y01 + ((y11 - y01) * (x - x0)) / (x1 - x0);
	if (y0 < 0) {
	    y0 = 0;
	}
	if (y1 >= vga_screen.height) {
	    y1 = vga_screen.height - 1;
	}
	int offset = y0 * vga_screen.width + x;
	for (int y = y0; y <= y1; y++) {
	    vga_screen.buffer[offset] = color;
	    offset += vga_screen.width;
	}
    }


#endif
}

void initialize_modex()
{
    write_regs(g_320x200x256_modex);


    vga_screen.width = 320;
    vga_screen.height = 200;
    vga_screen.start = 0;
    vga_screen.size = vga_screen.width * vga_screen.height;

    vga_screen.buffer = (uint8_t *) (512 * 1024 - 65536);
    vga_screen.front_buffer = (uint8_t *) 0xa0000;
    vga_screen.back_buffer = (uint8_t *) (0xa0000 + vga_screen.size / 4);
}

void set_color(int color, int red, int blue, int green)
{
    outportb(0x3C8, color);
    outportb(0x3C9, red);
    outportb(0x3C9, blue);
    outportb(0x3C9, green);
}

void vga_main()
{
    initialize_modex();

#if 0
    for (int i = 1; i < 256; i++) {
	unsigned int color = rand();
	set_color(i, color & 0x3f, (color >> 8) & 0x3f,
		  (color >> 16) & 0x3f);
    }
#else
    for (int i = 1; i < 252; i++) {
	unsigned int color = rand();
	set_color(i, color & 0x3f, (color >> 8) & 0x3f,
		  (color >> 16) & 0x3f);
	int x = i >> 2;
	int y = i & 3;
	set_color(i, x + (y > 0), x + (y > 1), x + (y > 2));
    }
    for (int i = 252; i < 256; i++) {
	set_color(i, 63, 63, 63);
    }

#endif
    clear_screen(0);

}

void flip_buffers()
{

    for (int i = 0; i < 4; i++) {
	set_plane_mask(1 << i);
	for (int j = 0; j < vga_screen.size / 4; j++) {
	    vga_screen.back_buffer[j] = vga_screen.buffer[j * 4 + i];
	}
    }

    volatile uint8_t *buffer = vga_screen.front_buffer;
    vga_screen.front_buffer = vga_screen.back_buffer;
    vga_screen.back_buffer = buffer;

    vga_screen.start = vga_screen.size / 4 - vga_screen.start;
    set_frame(vga_screen.start);
}

float ifs[][6] = {
    {0.14, 0.01, 0, 0.51, -0.08, -1.31},
    {0.43, 0.52, -0.45, 0.5, 1.49, -0.75},
    {0.45, -0.49, 0.47, 0.47, -1.62, -0.74},
    {0.49, 0, 0, 0.51, 0.02, 1.62}

};
float leaf[2] = { 0, 0 };

int buf[320 * 200];

void vga_update()
{

#if 0
    const int trapezoids = 1 + rand() % 1;
    //const int trapezoids = 1;
    for (int i = 0; i < trapezoids; i++)
	//for(;;) 
    {
	int x0 = rand() % vga_screen.width;
	int x1 = rand() % vga_screen.width;
	if (x0 > x1) {
	    x1 ^= x0;
	    x0 ^= x1;
	    x1 ^= x0;
	}
	int y00 = (rand() % (vga_screen.height)) - vga_screen.height * 0;
	int y01 = (rand() % (vga_screen.height)) - vga_screen.height * 0;
	if (y00 > y01) {
	    y01 ^= y00;
	    y00 ^= y01;
	    y01 ^= y00;
	}

	int y10 = (rand() % (vga_screen.height)) - vga_screen.height * 0;
	int y11 = (rand() % (vga_screen.height)) - vga_screen.height * 0;

	if (y10 > y11) {
	    y11 ^= y10;
	    y10 ^= y11;
	    y11 ^= y10;
	}

	const int color = (rand() * 0x01010101) >> 24;


	draw_trapezoid(x0, y00, y01, x1, y10, y11, color);
    }
#else
    for (int i = 0; i < 10000; i++) {
	int index = ((rand() * 0x55555555) >> 30) & 3;
	float x = leaf[0], y = leaf[1];
	leaf[0] = ifs[index][0] * x + ifs[index][1] * y + ifs[index][4];
	leaf[1] = ifs[index][2] * x + ifs[index][3] * y + ifs[index][5];
	x = leaf[0] / 8 + 0.5;
	y = -leaf[1] / 8 + 0.5;
	int sx = x * vga_screen.width;
	int sy = y * vga_screen.height;
	if (0 <= sx && sx < vga_screen.width && 0 <= sy
	    && sy < vga_screen.height) {
	    //vga_screen.buffer[sy*vga_screen.width+sx] = (rand() * 0x01010101) >> 24;
	    buf[sy * vga_screen.width + sx]++;
	}
    }
    int mx = 1;
    for (int i = 0; i < 320 * 200; i++) {
	if (mx < buf[i]) {
	    mx = buf[i];
	}
    }
    for (int i = 0; i < 320 * 200; i++) {
	double x = (double) buf[i] / (double) mx;
	x = 1.0 - x;
	x *= x;
	x *= x;
	x *= x;
	x *= x;
	x *= x;
	x *= x;
	x *= x;
	x = 1.0 - x;
	vga_screen.buffer[i] = 254 * x;
    }
#endif
    flip_buffers();
}
