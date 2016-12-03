#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdint.h>

#define PULLUP 		0x00
#define REPEATER	0x01
#define NEITHER 	0x02
#define PULLDOWN 	0x03

#define IN 0x00
#define OUT 0x01

void set_pin_function(uint8_t port, uint8_t pin, uint8_t function);
void set_pin_mode(uint8_t port, uint8_t pin, uint8_t mode);
void set_pin_od(uint8_t port, uint8_t pin, uint8_t od);
void set_pin_dir(uint8_t port, uint8_t pin, uint8_t dir);
void pin_set(uint8_t port, uint8_t pin);
void pin_clear(uint8_t port, uint8_t pin);

#endif
