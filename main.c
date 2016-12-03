#include "inc/LPC17xx.h"
#include "hdr/hdr_sc.h"
#include "gpio.h"
#include "uart.h"
#include "pll0.h"

static void system_init(void)
{
        LPC_SC->PCLKSEL0 = LPC_SC_PCLKSEL0_PCLK_WDT_DIV1 |LPC_SC_PCLKSEL0_PCLK_TIMER0_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_TIMER1_DIV1 | LPC_SC_PCLKSEL0_PCLK_UART0_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_UART1_DIV1 | LPC_SC_PCLKSEL0_PCLK_PWM1_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_I2C0_DIV1 | LPC_SC_PCLKSEL0_PCLK_SPI_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_SSP1_DIV1 | LPC_SC_PCLKSEL0_PCLK_DAC_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_ADC_DIV1 | LPC_SC_PCLKSEL0_PCLK_CAN1_DIV1 |
                        LPC_SC_PCLKSEL0_PCLK_CAN2_DIV1 | LPC_SC_PCLKSEL0_PCLK_ACF_DIV1;
        LPC_SC->PCLKSEL1 = LPC_SC_PCLKSEL1_PCLK_QEI_DIV1 | LPC_SC_PCLKSEL1_PCLK_GPIOINT_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_PCB_DIV1 | LPC_SC_PCLKSEL1_PCLK_I2C1_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_SSP0_DIV1 | LPC_SC_PCLKSEL1_PCLK_TIMER2_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_TIMER3_DIV1 | LPC_SC_PCLKSEL1_PCLK_UART2_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_UART3_DIV1 | LPC_SC_PCLKSEL1_PCLK_I2C2_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_I2S_DIV1 | LPC_SC_PCLKSEL1_PCLK_RIT_DIV1 |
                        LPC_SC_PCLKSEL1_PCLK_SYSCON_DIV1 | LPC_SC_PCLKSEL1_PCLK_MC_DIV1;

	uint32_t core_f = 0;

        set_pin_function(0, 0, 0x00);
        set_pin_mode(0, 0, NEITHER);
        set_pin_od(0, 0, 0);
        set_pin_dir(0, 0, OUT);

        set_pin_function(0, 1, 0x00);
        set_pin_mode(0, 1, NEITHER);
        set_pin_od(0, 1, 0);
        set_pin_dir(0, 1, OUT);

        pin_clear(0,0);
        pin_clear(0,1);

        pin_set(0,0);

        core_f = pll0_start(25000000, 120000000);

        pin_set(0,1);

        uart_init(core_f, 115200);

	uart_println("UART0 initialized");
	uart_print("CPU set to ");
	uart_print_int(core_f);
	uart_println(" Hz");
}

void delay()
{
	uint32_t count, count_max = 1000000;
	for(count = 0; count < count_max; count++);
}

int main()
{
	system_init();

	for(;;)
	{
		pin_set(0, 1);
		pin_clear(0, 1);

		uart_print_char(uart_read_char());
	}

	return 0;
}
