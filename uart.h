#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

void uart_init(uint32_t core_clock, uint32_t baud);
void uart_print_char(char c);
void uart_print(const char *str);
void uart_println(const char *str);
void uart_print_int(uint32_t i);

void uart_print_hex(char c);
void uart_print_hex16(uint16_t h);
void uart_print_hex32(uint32_t h);
void uart_print_hex_str(uint8_t *data, uint32_t len);

uint8_t uart_read_char();

#endif
