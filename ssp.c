#include "inc/LPC17xx.h"
#include "hdr/hdr_sc.h"
#include "ssp.h"
#include "common.h"
#include "gpio.h"
#include "uart.h"

#define PCSSP0		21

// CR0 register bits
#define TRANSFER8	0b0111
#define TRANSFER16 	0b1111
#define SPI_FRF		0x00
#define CPOL		6
#define CPHA		7
#define CLOCK_SHIFT	8

// CR1 register bits
#define SSE		1

// SR register bits
#define TFE		0


void ssp_init_spi(uint32_t freq)
{
	LPC_SC->PCONP |= BV(PCSSP0);
	LPC_SC->PCLKSEL1 |= LPC_SC_PCLKSEL1_PCLK_SSP0_DIV1;
	set_pin_function(0, 15, 0x2);
	//set_pin_function(0, 16, 0x2); // SSEL
	set_pin_function(0, 17, 0x2);
	set_pin_function(0, 18, 0x2);

	// CPOL = 0, CPHA = 1,

	LPC_SSP0->CR0 = TRANSFER8 | BV(CPHA) | (5 << CLOCK_SHIFT);

	LPC_SSP0->CPSR = 2; // Close enough for now...

	// Go
	LPC_SSP0->CR1 = BV(SSE);
	uart_println("SSP0 initialized for SPI");
}



