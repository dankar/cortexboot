#include "uart.h"
#include "gpio.h"
#include "common.h"
#include "inc/LPC17xx.h"

#define PCUART0 	3
#define FIFO_ENABLE	0


// LSR bits
#define RDR		0
#define THRE		5
#define DLAB 		7

// LCR bits
#define BITMASK8	3
#define STOP_BIT	2
#define PARITY		3

LPC_UART0_TypeDef *uart = LPC_UART0;

uint8_t uart_read_char()
{
	while(!check_bit8(&uart->LSR, RDR));
	return uart->RBR;
}

void uart_print_char(char c)
{
	while(!check_bit8(&uart->LSR, THRE));
	uart->THR = c;
}

void uart_print(const char *str)
{
	for(int i = 0; i < strlen(str); i++)
	{
		uart_print_char(str[i]);
	}
}

void uart_println(const char *str)
{
	uart_print(str);
	uart_print_char('\r');
	uart_print_char('\n');
}

void uart_print_int(uint32_t i)
{
	char str[32];
	uint8_t pos = 31;
	str[31] = '\0';

	if(i == 0)
	{
		str[--pos] = '0';
	}
	else
	{
		while(i)
		{
			str[--pos] = '0' + (i % 10);
			i = i / 10;
		}
	}

	uart_print(&str[pos]);
}

void uart_print_hex(char c)
{
	uint8_t low = c & 0xf;
	uint8_t high = (c >> 4) & 0xf;

	if(high >= 0xa)
		uart_print_char('a' + high-0xa);
	else
		uart_print_char('0' + high);

	if(low >= 0xa)
		uart_print_char('a' + low-0xa);
	else
		uart_print_char('0' + low);

}

void uart_print_hex_str(uint8_t *data, uint32_t len)
{
	for(int i = 0; i < len; i++)
	{
		uart_print_hex(data[i]);
	}
}

void uart_print_hex16(uint16_t h)
{
	uart_print_hex((h >> 8) & 0xff);
	uart_print_hex((h) & 0xff);
}

void uart_print_hex32(uint32_t h)
{
	uart_print_hex((h >> 24) & 0xff);
	uart_print_hex((h >> 16) & 0xff);
	uart_print_hex((h >> 8) & 0xff);
	uart_print_hex((h) & 0xff);
}

void uart_init(uint32_t core_clock, uint32_t baud)
{
	uint16_t divisor = 0;

	set_pin_function(0, 2, 0x1); // Set pins to RX/TX
	set_pin_function(0, 3, 0x1);

	set_pin_mode(0, 2, NEITHER);
	set_pin_mode(0, 3, NEITHER);

	set_bit32(&LPC_SC->PCONP, PCUART0);

	set_bit8(&uart->LCR, DLAB);

	baud = baud * 16;
	divisor = core_clock / baud;
	uart->DLL = divisor & 0xff;
	uart->DLM = (divisor >> 8) & 0xff;

	clear_bit8(&uart->LCR, DLAB);

	uart->LCR &= ~(BITMASK8);
	uart->LCR |= BITMASK8;	// 8 bit
	clear_bit8(&uart->LCR, PARITY);   // no party
	clear_bit8(&uart->LCR, STOP_BIT); // 1 stop bit

	set_bit8(&uart->FCR, FIFO_ENABLE);
}
