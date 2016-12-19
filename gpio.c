#include "inc/LPC17xx.h"
#include "common.h"
#include "uart.h"

#define PINSEL_MASK 	0x3
#define PINMODE_MASK 	0x3

uint8_t debug = 0;

void set_pin_debug(uint8_t enable)
{
	debug = enable;
}

LPC_GPIO_TypeDef *get_port(uint8_t port)
{
	switch(port)
	{
	case 0x0:
		return LPC_GPIO0;
	case 0x1:
		return LPC_GPIO1;
	case 0x2:
		return LPC_GPIO2;
	case 0x3:
		return LPC_GPIO3;
	case 0x4:
		return LPC_GPIO4;
	default:
		return 0x0;
	}
}

void set_pin_function(uint8_t port, uint8_t pin, uint8_t function)
{
	volatile uint32_t *pinsel;

	pinsel = &(&LPC_PINCON->PINSEL0)[port * 2 + pin / 16];

	if(pin >= 16)
		pin -= 16;

	if(debug)
	{
		uart_print("Anding register with ");
		uart_print_int(PINSEL_MASK << (pin * 2));
		uart_println("");
		uart_print("Oring register with ");
		uart_print_int(function << (pin * 2));
		uart_println("");
	}
	*pinsel &= ~(PINSEL_MASK << (pin * 2)); // Clear function
	*pinsel |= function << (pin * 2); // apply new function

	if(debug)
	{
		uart_print("Register: ");
		uart_print_int((uint32_t)pinsel);
		uart_print(", value: ");
		uart_print_int((uint32_t)*pinsel);
		uart_println("");
	}
}

void set_pin_mode(uint8_t port, uint8_t pin, uint8_t mode)
{
	volatile uint32_t *pinmode;

	pinmode = &(&LPC_PINCON->PINMODE0)[port * 2 + pin / 16];

	if(pin >= 16)
		pin -= 16;

	*pinmode &= ~(PINMODE_MASK << (pin * 2));
	*pinmode |= mode << (pin * 2);
}

void set_pin_od(uint8_t port, uint8_t pin, uint8_t od)
{
	volatile uint32_t *modeod;

	modeod = &(&LPC_PINCON->PINMODE_OD0)[port];
	*modeod &= ~BV(pin);
	*modeod |= od << pin;
}


void set_pin_dir(uint8_t port, uint8_t pin, uint8_t dir)
{
	LPC_GPIO_TypeDef *gpio = get_port(port);

	gpio->FIODIR &= ~BV(pin);
	gpio->FIODIR |= dir << pin;
}

void pin_set(uint8_t port, uint8_t pin)
{
	LPC_GPIO_TypeDef *gpio = get_port(port);
	gpio->FIOSET |= BV(pin);
}

void pin_clear(uint8_t port, uint8_t pin)
{
	LPC_GPIO_TypeDef *gpio = get_port(port);
        gpio->FIOCLR |= BV(pin);
}

uint8_t pin_read(uint8_t port, uint8_t pin)
{
	LPC_GPIO_TypeDef *gpio = get_port(port);

	return (gpio->FIOPIN & BV(pin)) == 0 ? 0 : 1;
}
