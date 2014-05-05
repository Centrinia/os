
#include "util.h"

void cpuid(uint32_t * out, uint32_t index)
{

    asm volatile ("cpuid":"=a" (out[0])
		  , "=b"(out[1])
		  , "=c"(out[2])
		  , "=d"(out[3])
		  :"0"(index)
	);
}
