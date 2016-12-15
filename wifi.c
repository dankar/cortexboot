#include "wifi.h"
#include "ssp.h"
#include "uart.h"
#include "gpio.h"
#include "common.h"
#include "inc/LPC17xx.h"

#include "cc3000/wlan.h"
#include "cc3000/hci.h"

#define EXTMODE0	0
#define EXTMODE1	1
#define EXTMODE2	2
#define EXTMODE3	3

#define ISE_EINT3	21

void wifi_select(uint8_t select)
{
	if(select)
	{
		uart_println("Selecting slave");
		pin_clear(0, 16);
	}
	else
	{
		uart_println("Deselecting slave");
		pin_set(0, 16);
	}
}

void wifi_vben(uint8_t vben)
{
	if(vben)
	{
		uart_println("Setting vben...");
		pin_set(1, 10);
	}
	else
	{
		uart_println("Desetting vben...");
		pin_clear(1, 10);
	}
}

long wifi_irq_status()
{
	uint8_t p = pin_read(2, 13);
	uart_print("Reading IRQ status: ");
	uart_print_int(p);
	uart_println(".");
	return p;
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
	wifi_vben(1);
}

volatile uint8_t wifi_irq_enabled = 1;

void IntSpiGPIOHandler(void);

void wifi_interrupt()
{
	if(!(LPC_SC->EXTINT & BV(EXTMODE3)))
	{
		uart_println("NOT OUR INTERRUPT!!!");
	}
	LPC_SC->EXTINT = BV(EXTMODE3); // Ack interrupt
	uart_println("Got wifi IRQ");
	if(!wifi_irq_enabled)
	{
		return;
	}

	IntSpiGPIOHandler();
}

void wifi_enable_irq()
{
	uart_println("Enabling interrupts");
	NVIC->ISER[0] = BV(ISE_EINT3); // enable the interrupt
}

void wifi_disable_irq()
{
	uart_println("Disabling interrupts");
	NVIC->ICER[0] = BV(ISE_EINT3); // disable interrupt
}

char *send_patch(unsigned long *length)
{
	*length = 0;
	return 0;
}

volatile unsigned long ulSmartConfigFinished, ulCC3000Connected,ulCC3000DHCP,
OkToDoShutDown, ulCC3000DHCP_configured;
volatile unsigned char ucStopSmartConfig;
volatile long ulSocket;

#define HCI_EVENT_MASK                                 (HCI_EVNT_WLAN_KEEPALIVE | HCI_EVNT_WLAN_UNSOL_INIT | HCI_EVNT_WLAN_ASYNC_PING_REPORT)
#define NETAPP_IPCONFIG_MAC_OFFSET                     (20)

void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length)
{
	uart_println("Callback...");
	if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE)
	{
        	ulSmartConfigFinished = 1;
        	ucStopSmartConfig     = 1;
		uart_println("Simple config done");
    	}

    	if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT)
    	{
    		ulCC3000Connected = 1;
		uart_println("Connected");
    	}

   	if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT)
    	{
        	ulCC3000Connected = 0;
        	ulCC3000DHCP      = 0;
        	ulCC3000DHCP_configured = 0;
		uart_println("Disconnected");
    	}

   	if(lEventType == HCI_EVNT_WLAN_UNSOL_DHCP)
    	{
        //
        // Notes: 
        // 1) IP config parameters are received swapped
        // 2) IP config parameters are valid only if status is OK, 
        // i.e. ulCC3000DHCP becomes 1
        //

        //
        // Only if status is OK, the flag is set to 1 and the 
        // addresses are valid.
        //
        	if ( *(data + NETAPP_IPCONFIG_MAC_OFFSET) == 0)
        	{
            		//sprintf((char*)pucCC3000_Rx_Buffer,"IP:%d.%d.%d.%d\f\r", data[3],data[2], data[1], data[0]);
			ulCC3000DHCP = 1;
			uart_println("DHCP done");
        	}
        	else
        	{
			ulCC3000DHCP = 0;
			uart_println("Lost dhcp");
		}
	}

	if (lEventType == HCI_EVENT_CC3000_CAN_SHUT_DOWN)
	{
		OkToDoShutDown = 1;
		uart_println("Can shutdown");
	}
}

void wifi_init()
{
	uint8_t irq;

	wifi_pin_setup();

	uart_println("init...");
	wlan_init(CC3000_UsynchCallback, send_patch, send_patch, send_patch, wifi_irq_status, wifi_enable_irq, wifi_disable_irq, wifi_vben);

	uart_println("start...");
	wlan_start(0);

	uart_println("Start done!");
	wifi_disable_irq();
	for(;;);

	uart_println("event mask...");
	wlan_set_event_mask(HCI_EVENT_MASK);

	uart_println("connect...");
	wlan_connect(WLAN_SEC_WPA2, "data", 4, NULL, "bajskorv123", 11);

	uart_println("done...");
}

