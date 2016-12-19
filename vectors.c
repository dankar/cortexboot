#include "wifi.h"
#include "usb.h"

static void __Default_Handler(void) __attribute__ ((interrupt));
static void __Default_Handler(void)
{
	while (1);
}

static void __gpio_handler(void) __attribute__ ((interrupt));
static void __gpio_handler(void)
{
	wifi_interrupt();
}

static void __usb_handler(void) __attribute__ ((interrupt));
static void __usb_handler(void)
{
	usb_interrupt();
}

void NMI_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void HardFault_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void MemManage_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void BusFault_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void UsageFault_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void __Reserved_0x1C_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void __Reserved_0x20_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void __Reserved_0x24_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void __Reserved_0x28_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void SVCall_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void DebugMon_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void __Reserved_0x34_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void PendSV_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void SysTick_Handler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void WDT_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void TIMER0_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void TIMER1_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void TIMER2_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void TIMER3_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void UART0_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void UART1_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void UART2_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void UART3_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void PWM1_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void I2C0_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void I2C1_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void I2C2_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void SPI_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void SSP0_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void SSP1_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void PLL0_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void RTC_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void EINT0_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void EINT1_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void EINT2_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void EINT3_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__gpio_handler")));
void ADC_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void BOD_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void USB_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__usb_handler")));
void CAN_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void DMA_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void I2S_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void ENET_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void RIT_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void MCPWM_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void QEI_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void PLL1_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void USBActivity_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));
void CANActivity_IRQHandler(void) __attribute__ ((interrupt, weak, alias("__Default_Handler")));

extern const char __main_stack_end;

void reset_handler(void);

void (* const vectors[])(void) __attribute__ ((section(".vectors"))) = {
		(void (*)(void))&__main_stack_end,	// Main stack end address
		reset_handler,						// Reset
		NMI_Handler,						// Non Maskable Interrupt
		HardFault_Handler,					// All class of fault
		MemManage_Handler,					// Cortex-M3 Memory Management Interrupt
		BusFault_Handler,					// Cortex-M3 Bus Fault Interrupt
		UsageFault_Handler,					// Cortex-M3 Usage Fault Interrupt
		__Reserved_0x1C_Handler,			// Reserved 0x1C
		__Reserved_0x20_Handler,			// Reserved 0x20
		__Reserved_0x24_Handler,			// Reserved 0x24
		__Reserved_0x28_Handler,			// Reserved 0x28
		SVCall_Handler,						// Cortex-M3 SV Call Interrupt
		DebugMon_Handler,					// Cortex-M3 Debug Monitor Interrupt
		__Reserved_0x34_Handler,			// Reserved 0x34
		PendSV_Handler,						// Cortex-M3 Pend SV Interrupt
		SysTick_Handler,					// Cortex-M3 System Tick Interrupt
		WDT_IRQHandler,						// Watchdog Timer Interrupt
		TIMER0_IRQHandler,					// Timer0 Interrupt
		TIMER1_IRQHandler,					// Timer1 Interrupt
		TIMER2_IRQHandler,					// Timer2 Interrupt
		TIMER3_IRQHandler,					// Timer3 Interrupt
		UART0_IRQHandler,					// UART0 Interrupt
		UART1_IRQHandler,					// UART1 Interrupt
		UART2_IRQHandler,					// UART2 Interrupt
		UART3_IRQHandler,					// UART3 Interrupt
		PWM1_IRQHandler,					// PWM1 Interrupt
		I2C0_IRQHandler,					// I2C0 Interrupt
		I2C1_IRQHandler,					// I2C1 Interrupt
		I2C2_IRQHandler,					// I2C2 Interrupt
		SPI_IRQHandler,						// SPI Interrupt
		SSP0_IRQHandler,					// SSP0 Interrupt
		SSP1_IRQHandler,					// SSP1 Interrupt
		PLL0_IRQHandler,					// PLL0 Lock (Main PLL) Interrupt
		RTC_IRQHandler,						// Real Time Clock Interrupt
		EINT0_IRQHandler,					// External Interrupt 0 Interrupt
		EINT1_IRQHandler,					// External Interrupt 1 Interrupt
		EINT2_IRQHandler,					// External Interrupt 2 Interrupt
		EINT3_IRQHandler,					// External Interrupt 3 Interrupt
		ADC_IRQHandler,						// A/D Converter Interrupt
		BOD_IRQHandler,						// Brown-Out Detect Interrupt
		USB_IRQHandler,						// USB Interrupt
		CAN_IRQHandler,						// CAN Interrupt
		DMA_IRQHandler,						// General Purpose DMA Interrupt
		I2S_IRQHandler,						// I2S Interrupt
		ENET_IRQHandler,					// Ethernet Interrupt
		RIT_IRQHandler,						// Repetitive Interrupt Timer Interrupt
		MCPWM_IRQHandler,					// Motor Control PWM Interrupt
		QEI_IRQHandler,						// Quadrature Encoder Interface Interrupt
		PLL1_IRQHandler,					// PLL1 Lock (USB PLL) Interrupt
		USBActivity_IRQHandler,				// USB Activity interrupt
		CANActivity_IRQHandler, 			// CAN Activity interrupt
};
