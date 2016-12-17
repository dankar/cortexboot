#include "wifi.h"
#include "ssp.h"
#include "uart.h"
#include "gpio.h"
#include "common.h"
#include "inc/LPC17xx.h"

#include "cc3000/wlan.h"
#include "cc3000/hci.h"
#include "cc3000/socket.h"

#define EXTMODE0	0
#define EXTMODE1	1
#define EXTMODE2	2
#define EXTMODE3	3

#define ISE_EINT3	21

int sockfd = 0;

void wifi_select(uint8_t select)
{
	if(select)
	{
		pin_clear(0, 16);
	}
	else
	{
		pin_set(0, 16);
	}
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

long wifi_irq_status()
{
	uint8_t p = pin_read(2, 13);
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
		printf("NOT OUR INTERRUPT!!!\n");
	}
	LPC_SC->EXTINT = BV(EXTMODE3); // Ack interrupt
	if(!wifi_irq_enabled)
	{
		return;
	}

	IntSpiGPIOHandler();
}

void wifi_enable_irq()
{
	NVIC->ISER[0] = BV(ISE_EINT3); // enable the interrupt
	if(!wifi_irq_status())
	{
		wifi_interrupt();
	}
}

void wifi_disable_irq()
{
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
volatile uint8_t host_connected = 0;

#define HCI_EVENT_MASK                                 (HCI_EVNT_WLAN_KEEPALIVE | HCI_EVNT_WLAN_UNSOL_INIT | HCI_EVNT_WLAN_ASYNC_PING_REPORT)
#define NETAPP_IPCONFIG_MAC_OFFSET                     (20)

void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length)
{
	if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE)
	{
        	ulSmartConfigFinished = 1;
        	ucStopSmartConfig     = 1;
    	}

    	if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT)
    	{
    		ulCC3000Connected = 1;
		printf("Associated (callback)\n");
    	}

   	if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT)
    	{
        	ulCC3000Connected = 0;
        	ulCC3000DHCP      = 0;
        	ulCC3000DHCP_configured = 0;
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
            		printf("IP:%d.%d.%d.%d\n", data[3],data[2], data[1], data[0]);
			ulCC3000DHCP = 1;
			printf("DHCP done\n");
        	}
        	else
        	{
			ulCC3000DHCP = 0;
			printf("Lost dhcp\n");
		}
	}

	if (lEventType == HCI_EVENT_CC3000_CAN_SHUT_DOWN)
	{
		OkToDoShutDown = 1;
	}
}

uint8_t wifi_is_configured()
{
	return ulCC3000Connected && ulCC3000DHCP;
}

void wifi_init()
{
	uint8_t irq;

	wifi_pin_setup();
	host_connected = 0;

	printf("init...\n");
	wlan_init(CC3000_UsynchCallback, send_patch, send_patch, send_patch, wifi_irq_status, wifi_enable_irq, wifi_disable_irq, wifi_vben);

	printf("start...\n");
	wlan_start(0);

	printf("Start done!\n");

	printf("event mask...\n");
	wlan_set_event_mask(HCI_EVENT_MASK);

	printf("connect...\n");
	if(wlan_connect(WLAN_SEC_WPA2, "data", 4, NULL, "bajskorv123", 11) != 0)
	{
		printf("Could not associate\n");
		return;
	}
	else
	{
		while(!wifi_is_configured());
		printf("Associated with AP\n");
	}
}

uint8_t wifi_connect()
{
	if(!wifi_is_configured())
	{
		printf("Can't connect, wifi not associated\n");
	}
	host_connected = 0;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("Couldn't get socket\n");
		return 0;
	}
	printf("Got socket: %d\n", sockfd);

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5000);
	addr.sin_addr.s_addr = 0xA801A8C0; //C0A801A8;
	printf("Connecting to 192.168.1.168...\n");

	if(connect(sockfd, (sockaddr*)&addr, sizeof(addr)) != 0)
	{
		printf("Failed to connect.\n");
		return 0;
	}

	printf("Connected to host\n");

	host_connected = 1;
	return 1;
}

uint8_t wifi_send(const uint8_t *data, uint32_t len)
{
	uint32_t sent = 0;
	//if(!wifi_is_configured() || !host_connected)
	///{
	///	printf("Not connected, can't send\n");
	////	return 0;
	//}

	while(sent < len)
	{
		uint32_t result = send(sockfd, data+sent, len-sent, 0);
		if(result <= 0)
		{
			printf("Host lost\n");
			host_connected = 0;
			return 0;
		}

		sent += result;
	}

	return 1;
}

uint8_t wifi_recv(uint8_t *data, uint32_t len)
{
	uint32_t recvd = 0;
	//if(!wifi_is_configured() || !host_connected)
	//{
	//	printf("Not connected, can't recieve\n");
	//	return 0;
	//}

	while(recvd < len)
	{
		uint32_t result = recv(sockfd, data+recvd, len-recvd, 0);

		if(result <= 0)
		{
			printf("Host lost\n");
			host_connected = 0;
			return 0;
		}
		recvd += result;
	}

	return 1;
}
