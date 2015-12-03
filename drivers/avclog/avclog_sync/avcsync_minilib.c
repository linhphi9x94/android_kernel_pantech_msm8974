#include <asm/page.h>
#include <linux/kernel.h>
#include "avcsync_minilib.h"

void assert_fail(const char* s, const char* fname, int fline)
{
    PRINTF("\n AVCLOG ASSERTION Failed:\n"
	   "exp: %s, %s [L:%d]\n\n", s, fname, fline);
    // BUG();
}
