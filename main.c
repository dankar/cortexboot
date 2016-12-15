#include "inc/LPC17xx.h"
#include "hdr/hdr_sc.h"
#include "gpio.h"
#include "uart.h"
#include "pll0.h"
#include "iap.h"
#include "usb.h"
#include "usb_impl.h"
#include "ssp.h"
#include "common.h"
#include "wifi.h"

static void system_init(void)
{
        LPC_SC->PCLKSEL0 = LPC_SC_PCLKSEL0_PCLK_WDT_DIV1;
			/*LPC_SC_PCLKSEL0_PCLK_TIMER0_DIV1 |LPC_SC_PCLKSEL0_PCLK_TIMER1_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_UART1_DIV1 | LPC_SC_PCLKSEL0_PCLK_PWM1_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_I2C0_DIV1 | LPC_SC_PCLKSEL0_PCLK_SPI_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_SSP1_DIV1 | LPC_SC_PCLKSEL0_PCLK_DAC_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_ADC_DIV1 | LPC_SC_PCLKSEL0_PCLK_CAN1_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_CAN2_DIV1 | LPC_SC_PCLKSEL0_PCLK_ACF_DIV1;*/
        LPC_SC->PCLKSEL1 = LPC_SC_PCLKSEL1_PCLK_PCB_DIV1 | LPC_SC_PCLKSEL1_PCLK_SYSCON_DIV1 | LPC_SC_PCLKSEL1_PCLK_GPIOINT_DIV1;
			/*LPC_SC_PCLKSEL1_PCLK_QEI_DIV1 | LPC_SC_PCLKSEL1_PCLK_GPIOINT_DIV1 |
		        LPC_SC_PCLKSEL1_PCLK_PCB_DIV1 | LPC_SC_PCLKSEL1_PCLK_I2C1_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_SSP0_DIV1 | LPC_SC_PCLKSEL1_PCLK_TIMER2_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_TIMER3_DIV1 | LPC_SC_PCLKSEL1_PCLK_UART2_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_UART3_DIV1 | LPC_SC_PCLKSEL1_PCLK_I2C2_DIV1 |
                        /LPC_SC_PCLKSEL1_PCLK_I2S_DIV1 | LPC_SC_PCLKSEL1_PCLK_RIT_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_SYSCON_DIV1 | LPC_SC_PCLKSEL1_PCLK_MC_DIV1;*/

	uint32_t core_f = 0;

        set_pin_function(0, 0, 0x00);
        set_pin_mode(0, 0, NEITHER);
        set_pin_od(0, 0, 1);
        set_pin_dir(0, 0, OUT);

        set_pin_function(0, 1, 0x00);
        set_pin_mode(0, 1, NEITHER);
        set_pin_od(0, 1, 1);
        set_pin_dir(0, 1, OUT);

        pin_set(0,0);
	pin_set(0,1);

        core_f = pll0_start();

        pin_set(0,1);

        uart_init(core_f, 115200);

	uart_println("UART0 initialized");
	uart_print("CPU set to ");
	uart_print_int(core_f);
	uart_println(" Hz");

	//usb_init();
	ssp_init_spi(16000000);
	wifi_init();
}

#define LEFT_CONTROL	0
#define LEFT_SHIFT 	1
#define LEFT_ALT	2
#define LEFT_GUI	3
#define RIGHT_CONTROL	4
#define RIGHT_SHIFT	5
#define RIGHT_ALT	6
#define RIGHT_GUI	7

uint32_t key_age[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

uint8_t ascii_to_scancode(uint8_t ascii)
{
	ascii = to_lower(ascii);

	if(is_alpha(ascii))
	{
		return 0x04 + (ascii - 'a');
	}
	else if(is_numeric(ascii))
	{
		if(ascii == '0')
			return 0x27;
		else
			return 0x1E + (ascii - '1');
	}
	else
	{
		switch(ascii)
		{
		case 13:
			return 0x28;
		case '\t':
			return 0x2b;
		case ' ':
			return 0x2c;
		default:
			return 0;
		}
	}
}

void push_key(uint8_t ascii)
{
	uint8_t found = 0;
	int i;
	uint8_t scan_code;
	uint8_t upper = is_upper(ascii);

	scan_code = ascii_to_scancode(ascii);

	for(i = 2; i < 8; i++)
	{
		if(keyboard_buffer[i] == 0 || keyboard_buffer[i] == scan_code)
		{
			found = 1;
			break;
		}
	}

	if(!found)
		return;

	if(upper)
		keyboard_buffer[0] = BV(LEFT_SHIFT); // Modifiers
	else
		keyboard_buffer[0] = 0;

	pin_clear(0, 0);
	keyboard_buffer[i] = scan_code;
	key_age[i] = 20000;
}

void update_keys()
{
	for(int i = 2; i < 8; i++)
	{
		if(key_age[i])
		{
			key_age[i]--;
			if(key_age[i] == 0)
			{
				keyboard_buffer[i] = 0;
			}
		}
	}

	for(int i = 2; i < 8; i++)
		if(key_age[i])
			return;

	keyboard_buffer[0] = 0; // Modifiers
	pin_set(0,0);
}

int main()
{
	system_init();
	char c;
	pin_set(0, 0);
	pin_set(0, 1);
	uint32_t counter = 0;

	for(;;)
	{
//		usb_poll();
		counter++;
		if(counter % 1000000 == 0)
			uart_println("tick");

		if(uart_char_is_available())
		{
			char c = uart_read_char();
			push_key(c);
		}

		update_keys();
	}

	return 0;
}
