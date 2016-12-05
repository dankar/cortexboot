#ifndef _PLL0_H_
#define _PLL0_H_

#include <stdint.h>

void pll0_feed();

// Sets pll0 to 480MHz and core clk divisor to yield 120MHz core clock
uint32_t pll0_start();

#endif
