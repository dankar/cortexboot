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
#define TNF		1
#define RNE		2


void ssp_init_spi()
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

	for(int i = 0; i < 1024; i++)
	{
		uint16_t t = LPC_SSP0->DR;
	}
	uart_println("SSP0 initialized for SPI");
	//spi_send_data("aaaaaaaaaaaaaaaaaaaaaa", 15);
	//for(;;);
}

uint8_t spi_send(uint8_t data)
{
	uint8_t response;
	while(!(LPC_SSP0->SR & BV(TNF)));
	LPC_SSP0->DR = data;
	while(!(LPC_SSP0->SR & BV(RNE)));
 	response = (uint8_t)(LPC_SSP0->DR & 0xff);
	uart_print("Got response: ");
	uart_print_hex32(response);
	uart_println("");
	return response;
}

void spi_send_data(uint8_t *data, uint32_t len)
{
	uint8_t r;
	uart_print("Sending data: ");
        uart_print_hex_str(data, len);
        uart_println(" (not saving response)");
	for(int i = 0; i < len; i++)
	{
		while(!(LPC_SSP0->SR & BV(TNF)));
		LPC_SSP0->DR = data[i];
		while(!(LPC_SSP0->SR & BV(RNE)));
 		r = LPC_SSP0->DR;
	}
}

void spi_send_receive(uint8_t *data, uint32_t len)
{
	uart_print("Sending data: ");
	uart_print_hex_str(data, len);
	uart_println("");
	for(int i = 0; i < len; i++)
	{
		while(!(LPC_SSP0->SR & BV(TNF)));
		LPC_SSP0->DR = data[i];
		while(!(LPC_SSP0->SR & BV(RNE)));
 		data[i] = LPC_SSP0->DR;
	}
	uart_print("Response: ");
	uart_print_hex_str(data, len);
	uart_println("");
}
