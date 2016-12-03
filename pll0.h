#ifndef _PLL0_H_
#define _PLL0_H_

#include <stdint.h>

void pll0_feed();
uint32_t pll0_start(uint32_t crystal, uint32_t frequency);

#endif
