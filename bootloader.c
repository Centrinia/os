


#define STACK_ADDRESS 0x7a00

#define NORETURN __attribute__((noreturn))
#define NOINLINE __attribute__((noinline))
#define REGPARM(n) __attribute__((regparm(n)))


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#if defined(__GNUC__)
//__asm__(".code16gcc\n\t");
#endif

static short read_sector(void *dst, const int drive, const int cylinder,
                         const int head, const int sector)
{
    short failed = 0, ax;
    __asm__ volatile ("int $0x13\n\t"
                      "sbb %0, %0"
                      :"=r" (failed), "=a"(ax)
                      :"a"(0x0201),
                      "c"(((cylinder & 0xff) << 8) |
                          ((cylinder & 0x300) >> 2) | (sector & 0x3f)),
                      "d"((head << 8) | drive), "b"(dst)
                      :"cc");
    if (failed) {
        return ax;
    }
    return ax & 0xff;
}

short NOINLINE REGPARM(3)
read_sectors(void *dst, const uint16_t drive, const uint32_t lba_start,
             const short count)
{
    short sectors_per_track, heads_per_cylinder;
    short i;
    short error = 0;

    {
        short failed, e;
        uint16_t cx;
        uint16_t dx;

        __asm__ volatile ("int $0x13\n\t"
                          "sbb %0,%0"
                          :"=r" (failed), "=c"(cx), "=d"(dx),
                          "=a"(e)
                          :"a"(0x0800), "d"(drive), "D"(0)
                          :"cc", "bx");


        heads_per_cylinder = ((dx >> 8) & 0xff) + 1;
        sectors_per_track = cx & 0x3f;

        if (failed) {
            return (e >> 8) & 0xff;
        }

    }

    for (i = 0; i < count; i++) {
        uint32_t lba = lba_start + i;
        short cylinder, head, sector, track;
        track = lba / sectors_per_track;
        sector = (lba % sectors_per_track) + 1;
        head = track % heads_per_cylinder;
        cylinder = track / heads_per_cylinder;
        error |=
            !read_sector(dst + i * 512, drive, cylinder, head, sector);
    }
    return error;
}

int main() {
	read_sectors(0,1,2,3);
}


