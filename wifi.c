#include "wifi.h"
#include "ssp.h"
#include "uart.h"
#include "gpio.h"
#include "common.h"
#include "inc/LPC17xx.h"

#define EXTMODE0	0
#define EXTMODE1	1
#define EXTMODE2	2
#define EXTMODE3	3

#define ISE_EINT3	21

void wifi_select(uint8_t select)
{
	if(select)
		pin_clear(0, 16);
	else
		pin_set(0, 16);
}

void wifi_vben(uint8_t vben)
{
	if(vben)
	{
		pin_set(1, 10);
	}
	else
	{
		pin_clear(1, 10);
	}
}

uint8_t wifi_irq_status()
{
	return pin_read(2, 13);
}

void wifi_pin_setup()
{
	set_pin_function(1, 10, 0x00); // VBEN
	set_pin_function(2, 13, 0x01); // WIFI_IRQ
	set_pin_function(0, 16, 0x00); // SSEL

	set_pin_dir(1, 10, OUT);
	//set_pin_dir(2, 13, IN);
	set_pin_dir(0, 16, OUT);

	set_pin_mode(2, 13, PULLUP);

	NVIC->ICER[0] = BV(ISE_EINT3); // disable interrupt
	LPC_SC->EXTMODE = BV(EXTMODE3); // Set edge detection
	LPC_SC->EXTINT = BV(EXTMODE3); // Ack interrupt
	NVIC->ISER[0] = BV(ISE_EINT3); // enable the interrupt

	wifi_select(0);
	wifi_vben(0);
	uart_println("Waiting...");
	sleep();
	sleep();
	sleep();
	sleep();
	sleep();
}

volatile uint8_t command_in_progress = 0;

void wifi_interrupt()
{
	if(!(LPC_SC->EXTINT & BV(EXTMODE3)))
	{
		uart_println("NOT OUR INTERRUPT!!!");
	}
	LPC_SC->EXTINT = BV(EXTMODE3); // Ack interrupt
	uart_println("Got wifi IRQ");
	if(command_in_progress)
	{
		uart_println("Currently sending command, ignoring");
	}
}

void wifi_init()
{
	uint8_t irq;

	wifi_pin_setup();

	irq = wifi_irq_status();

	wifi_vben(1);

	if(irq)
	{
		uart_println("IRQ was high, waiting for it to go low...");
		while(wifi_irq_status());
	}
	else
	{
		uart_println("IRQ was low, waiting for it to go high...");
		while(!wifi_irq_status());
		uart_println("...and now low");
		while(wifi_irq_status());
	}

	wifi_select(1);
	command_in_progress = 1;
	sleep(); // Should "at least 50us"

	uint8_t command[] = {0x01, 0x00, 0x05, 0x00};
	spi_send(command, 4);
	uart_print("Response: ");
	uart_print_hex_str(command, 4);
	uart_println("");
	sleep();
	uint8_t data[] = { 0x00, 0x01, 0x00, 0x40, 0x01, 0x00 };
	spi_send(data, 6);
	uart_print("Response: ");
	uart_print_hex_str(data, 6);
	uart_println("");
	command_in_progress = 0;
	wifi_select(0);

	uart_println("Done!");
}

