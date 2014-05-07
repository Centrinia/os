/* stdlib.c */

#include "os.h"

void *memset(void *s, int c, size_t n)
{
#if 0
    for (size_t i = 0; i < n; i++) {
	((char *) s)[i] = c;
    }
#else
    asm volatile ("cld\n\trep stosb"::"D" (s), "a"(c), "c"(n));
#endif
    return s;
}
