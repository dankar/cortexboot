#include "inc/LPC17xx.h"
#include "gpio.h"
#include "common.h"
#include "uart.h"
#include "usb_proto.h"
#include "usb_device.h"
#include "usb_msd.h"
#include "usb.h"

// ISER0 bits
#define ISE_USB		24

// USBRxPLen register bits
#define DV		10
#define PKT_RDY		11

// PCONP register bits
#define PCUSB		31

// USBCtrl bits
#define RD_EN		0
#define WR_EN		1

// USBClkCtrl bits
#define DEV_CLK_EN	1
#define AHB_CLK_EN	4

// USBDevInt registers bits
#define FRAME		0
#define EP_FAST		1
#define EP_SLOW		2
#define DEV_STAT	3
#define CCEMPTY		4
#define CDFULL		5
#define RXENDPKT	6
#define TXENDPKT	7
#define EP_RLZED	8
#define ERR_INT		9

// Endpoint status bits
#define STP 		2

// Command clear buffer
#define CMD_CLEAR_BUFFER	0xF2
// Command validate data
#define CMD_VALIDATE_BUFFER	0xFA
// configure
#define CMD_CONFIGURE_DEVICE    0xD8

// Set Mode command
#define CMD_SET_MODE 	0xF3
// Set mode bits
#define AP_CLK		0
#define INAK_CI		1
#define INAK_CO		2

// USBEpIntSt register bits

#define EP0RX		0
#define EP0TX		1
#define EP1RX		2
#define EP1TX		3
#define EP2RX		4
#define EP2TX		5
#define EP4RX		8
#define EP4TX		9


// Set address command
#define CMD_SET_ADDRESS	0xD0
// Set address bits
#define DEV_EN		7

// Set device status command
#define CMD_SET_STATUS	0xFE
// Set status bits
#define CON		0

// Endpoint status bits
#define EP_STATUS_ST	0

uint8_t *control_data_to_send = 0;
uint8_t control_num_to_send = 0;
uint8_t control_num_sent = 0;

uint8_t *control_data_to_receive = 0;
uint8_t control_num_to_receive = 0;
uint8_t control_num_received = 0;

uint8_t *bulk_data_to_send = 0;
uint32_t bulk_num_to_send = 0;
uint32_t bulk_num_sent = 0;

uint8_t *bulk_data_to_receive = 0;
uint32_t bulk_num_to_receive = 0;
uint32_t bulk_num_received = 0;

uint8_t usb_endpoint_to_phy(uint8_t endpoint)
{
        return ((endpoint & ~0x80) << 1) + ((endpoint & 0x80) ? 1 : 0);
}

void usb_sie_command(uint8_t command, uint8_t data)
{
	LPC_USB->USBDevIntClr = BV(CCEMPTY); // Clear CCEMPTY
	LPC_USB->USBCmdCode = (command << 16) | 0x0500;
	while(!(LPC_USB->USBDevIntSt & BV(CCEMPTY)));
	LPC_USB->USBDevIntClr = BV(CCEMPTY);
	LPC_USB->USBCmdCode = (data << 16) | 0x0100;
	while(!(LPC_USB->USBDevIntSt & BV(CCEMPTY)));
	LPC_USB->USBDevIntClr = BV(CCEMPTY);
}

void usb_sie_command_nd(uint8_t command)
{
	LPC_USB->USBDevIntClr = BV(CCEMPTY); // Clear CCEMPTY
	LPC_USB->USBCmdCode = (command << 16) | 0x0500;
	while(!(LPC_USB->USBDevIntSt & BV(CCEMPTY)));
	LPC_USB->USBDevIntClr = BV(CCEMPTY);
}

void usb_sie_command_ep_nd(uint32_t endpoint, uint8_t command)
{
	LPC_USB->USBDevIntClr = BV(CCEMPTY);
	LPC_USB->USBCmdCode = (endpoint << 16) | 0x0500;
	while(!(LPC_USB->USBDevIntSt & BV(CCEMPTY)));
	LPC_USB->USBDevIntClr = BV(CCEMPTY);
	LPC_USB->USBCmdCode = (command << 16) | 0x0500;
	while(!(LPC_USB->USBDevIntSt & BV(CCEMPTY)));
	LPC_USB->USBDevIntClr = BV(CCEMPTY);
}

void usb_realize_endpoint(uint32_t endpoint, uint32_t max_packet_size)
{
	LPC_USB->USBReEp |= 1 << endpoint;
	LPC_USB->USBEpInd = endpoint;
	LPC_USB->USBMaxPSize = max_packet_size;

	printf("Waiting for realization of endpoint %d...\n", endpoint);

	while(!(LPC_USB->USBDevIntSt & BV(EP_RLZED)));

	LPC_USB->USBDevIntClr |= BV(EP_RLZED);

	printf("Current endpoints: %x\n", LPC_USB->USBReEp);
}

void usb_enable_endpoint_interrupt(uint32_t endpoint)
{
	LPC_USB->USBEpIntEn |= BV(endpoint);
}




uint32_t usb_init()
{
	set_bit32(&LPC_SC->PCONP, PCUSB); // Enable USB power

	LPC_SC->USBCLKCFG = 0x09; // pll0 = 480MHz, divisor 10 gives 48MHz

	LPC_USB->USBClkCtrl |= BV(DEV_CLK_EN) | BV(AHB_CLK_EN); // Enable clocks

	printf("USB clock status: ");
	while(!((LPC_USB->USBClkSt & (BV(DEV_CLK_EN)|BV(AHB_CLK_EN))) == (BV(DEV_CLK_EN)|BV(AHB_CLK_EN)))); // Wait for clock status OK

	printf("ok\n");

	// set_pin_debug(1);

	set_pin_function(0, 29, 0x01); // Set USB D-/D+ function
	set_pin_function(0, 30, 0x01);

	set_pin_function(1, 18, 0x01); // USB_UP_LED
	set_pin_function(1, 30, 0x02); // VBUS

	//set_pin_mode(1, 30, NEITHER);
	//set_pin_mode(1, 18, NEITHER);
	//set_pin_mode(0, 29, NEITHER);
	//set_pin_mode(0, 30, NEITHER);

	LPC_USB->USBDevIntEn = BV(EP_SLOW) | BV(DEV_STAT) | BV(EP_FAST) | BV(ERR_INT);

	LPC_USB->USBDevIntClr |= BV(EP_RLZED);

#ifdef VERBOSE_DEBUG
	printf("Current value of realized endpoints: %x\n", LPC_USB->USBReEp);
#endif
	//usb_realize_endpoint(usb_endpoint_to_phy(CONTROL_OUT_ENDPOINT), MAX_PACKET_SIZE0);
	//usb_realize_endpoint(usb_endpoint_to_phy(CONTROL_IN_ENDPOINT), MAX_PACKET_SIZE0);

	usb_configure_endpoint(CONTROL_OUT_ENDPOINT, MAX_PACKET_SIZE0);
	usb_configure_endpoint(CONTROL_IN_ENDPOINT, MAX_PACKET_SIZE0);

#ifdef VERBOSE_DEBUG
	printf("Control endpoints are now realized\n");
#endif
	LPC_USB->USBEpIntClr = 0xffffffff; // Clear all interrupts
	LPC_USB->USBDevIntClr = 0xffffffff;

	//usb_enable_endpoint_interrupt(usb_endpoint_to_phy(CONTROL_OUT_ENDPOINT));
	//usb_enable_endpoint_interrupt(usb_endpoint_to_phy(CONTROL_IN_ENDPOINT));
#ifdef VERBOSE_DEBUG
	printf("Enabled interrupts\n");
#endif
	usb_sie_command(CMD_SET_MODE, BV(AP_CLK) /*| BV(INAK_CI) | BV(INAK_CO)*/);

	printf("Set AP_CLK to 1\n");

	usb_sie_command(CMD_SET_ADDRESS, 0x00 |BV(DEV_EN));

	usb_sie_command(CMD_SET_STATUS, BV(CON));

	uint32_t *prioreg = 0xe000e418;
	*prioreg |= 5 << 3;
	NVIC->ISER[0] = BV(ISE_USB);
}

void usb_clear_endpoint_interrupt(uint8_t ep)
{
	LPC_USB->USBEpIntClr = ep;

	while(!(LPC_USB->USBDevIntSt & BV(CDFULL)));
	LPC_USB->USBDevIntClr = BV(CDFULL);
}

uint32_t usb_read_endpoint(uint32_t endpoint, uint8_t *data)
{
	uint32_t packet_len;
	LPC_USB->USBCtrl = ((endpoint & 0xf) << 2) | BV(RD_EN);

	do
	{
		packet_len = LPC_USB->USBRxPLen;

		//uart_print_hex32(packet_len);
		//uart_println("");
	} while(!(packet_len & (BV(PKT_RDY))));

	packet_len &= ~(BV(PKT_RDY) | BV(DV));

	//uart_print_int(packet_len);
	///uart_println(" bytes ready to be read");

	for(int i = 0; i < packet_len; i+=4)
	{
		*(uint32_t*)data = LPC_USB->USBRxData;
		data+=4;
	}

	LPC_USB->USBCtrl = 0;
	usb_sie_command_nd(CMD_CLEAR_BUFFER);

	return packet_len;
}

void usb_write_endpoint(uint32_t endpoint, uint8_t *data, uint32_t count)
{
	LPC_USB->USBCtrl = ((endpoint & 0x0f) << 2) | BV(WR_EN);

	LPC_USB->USBTxPLen = count;

	if(count == 0)
	{
		//uart_println("Sending status message");
	}

	for(int i = 0; i < count; i+=4)
	{
		//uart_print("SENDING DATA: ");
		//uart_print_hex32(*(uint32_t*)data);
		//uart_println("");
		LPC_USB->USBTxData = *(uint32_t*)data;
		data+=4;
	}
	LPC_USB->USBCtrl = 0;
	usb_sie_command_ep_nd((endpoint << 1) + 1, CMD_VALIDATE_BUFFER);
}

void usb_bulk_data_out()
{
	uint32_t len = bulk_num_to_receive - bulk_num_received;
	uint32_t read = 0;

	if(len > 64)
		len = 64;

	read = usb_read_endpoint(BULK_OUT_ENDPOINT, bulk_data_to_receive + bulk_num_received);
	bulk_num_received += read;

	if(bulk_num_received == bulk_num_to_receive)
	{
		if(!usb_msd_wants_more_data(bulk_data_to_receive, 512))
		{
			bulk_data_to_receive = 0;
			bulk_num_received = 0;
			bulk_num_to_receive = 0;
		}
	}
}

void usb_bulk_data_in()
{
	uint32_t len = bulk_num_to_send - bulk_num_sent;

	if(len > 64)
		len = 64;

	//printf("Sending %d, %d remaining\n", len, bulk_num_to_send - bulk_num_sent);
	usb_write_endpoint(BULK_IN_ENDPOINT, bulk_data_to_send + bulk_num_sent, len);

	bulk_num_sent += len;

	if(bulk_num_sent == bulk_num_to_send)
	{
		if(!usb_msd_has_more_data())
		{
			bulk_data_to_send = 0;
			bulk_num_sent = 0;
			bulk_num_to_send = 0;
		}
	}
}

void usb_data_in_stage()
{
	uint32_t len = control_num_to_send - control_num_sent;

	if(len > MAX_PACKET_SIZE0)
		len = MAX_PACKET_SIZE0;

	usb_write_endpoint(CONTROL_IN_ENDPOINT, control_data_to_send + control_num_sent, len);

	control_num_sent += len;

	if(control_num_sent == control_num_to_send)
	{
		control_data_to_send = 0;
		control_num_sent = 0;
		control_num_to_send = 0;
	}
}
#define SETUP_STAGE	0
#define IN_STAGE	1
#define OUT_STAGE	2

void usb_bulk(uint32_t stage)
{
	uint8_t bulk_data[64] = {0};
	uint32_t num_recv;

	switch(stage)
	{
	case IN_STAGE:
		if(bulk_data_to_send)
		{
			usb_bulk_data_in();
		}
		else if(msd_has_status)
		{
			usb_msd_status();
			if(bulk_data_to_send)
			{
				usb_bulk_data_in();
			}
		}
		break;
	case OUT_STAGE:
		// If a write command is active we get the next sector
		if(usb_msd_write_active())
		{
			usb_bulk_data_out();
		}
		else
		{
			// Get new scsi command
			bulk_data_to_send = 0; bulk_num_to_send = 0; bulk_num_sent = 0;
			num_recv = usb_read_endpoint(BULK_OUT_ENDPOINT, bulk_data);
			usb_msd_out(bulk_data, num_recv);
			if(bulk_data_to_send)
			{
				usb_bulk_data_in();
			}
			break;
		}
	};
}

void usb_endpoint0(uint32_t stage)
{
	uint8_t control_request[MAX_PACKET_SIZE0] = {0};
	uint32_t num_recv = 0;

	switch(stage)
	{
	case SETUP_STAGE:
		control_data_to_send = 0; control_num_to_send = 0; control_num_sent = 0;
		num_recv = usb_read_endpoint(0x00, control_request);
		usb_control_request(control_request, num_recv, 0, 0);
		if(control_data_to_send)
		{
			usb_data_in_stage();
		}
		else
		{
			//uart_println("No data to send...");
		}
		break;
	case IN_STAGE:
		if(current_control_request.request_type & CONTROL_DEVICE_TO_HOST)
		{
			usb_data_in_stage();
		}
		else
		{
			if(usb_device_address)
			{
				usb_set_address(usb_device_address);
			}
		}
		break;
	case OUT_STAGE:
		if(current_control_request.request_type & CONTROL_DEVICE_TO_HOST)
		{
			usb_read_endpoint(0x00, control_request); // get status
		}
		else
		{
			if(control_data_to_receive)
			{
				control_num_received += usb_read_endpoint(0x00, control_data_to_receive + control_num_received);
				if(control_num_received >= control_num_to_receive)
				{
					usb_control_request(0, 0, control_data_to_receive, control_num_received);
				}
			}
			else
			{
				printf("ERROR! GOT EXTRA DATA\n");
			}
		}
	}
}

uint32_t usb_set_clear_and_status(uint16_t endpoint)
{
	LPC_USB->USBEpIntClr = BV(endpoint); // Select endpoint and clear interrupt
        while((LPC_USB->USBDevIntSt & BV(CDFULL)) == 0);
	return LPC_USB->USBCmdData;
}

void usb_poll()
{
	uint32_t devintst = 0;
	if(!(devintst = LPC_USB->USBDevIntSt))
	{
		return;
	}

	if(devintst & BV(EP_SLOW))
	{
		uint32_t epint = LPC_USB->USBEpIntSt;

		if(epint & BV(EP0RX))
		{
			uint32_t status = usb_set_clear_and_status(EP0RX);

			if(status & BV(STP))
			{
				// Setup a new control transfer
				usb_endpoint0(SETUP_STAGE);
			}
			else
			{
				usb_endpoint0(OUT_STAGE);
			}

			epint &= ~BV(EP0RX);
		}

		if(epint & BV(EP0TX))
		{
			usb_set_clear_and_status(EP0TX);
			usb_endpoint0(IN_STAGE);
			epint &= ~BV(EP0TX);
		}

		if(epint & BV(EP1TX))
		{
			usb_set_clear_and_status(EP1TX);
			usb_endpoint_interrupt();
			epint &= ~BV(EP1TX);
		}
		if(epint & BV(EP1RX))
		{
			sleep();
			epint &= ~BV(EP1RX);
		}
		if(epint & BV(EP2RX))
		{
			usb_set_clear_and_status(EP2RX);
			usb_bulk(OUT_STAGE);
			epint &= ~BV(EP2RX);
		}
		if(epint & BV(EP2TX))
		{
			usb_set_clear_and_status(EP2TX);
			usb_bulk(IN_STAGE);
			epint &= ~BV(EP2TX);
		}
		if(epint)
		{
			printf("Unhandled endpoint interrupt: %x\n", epint);
			sleep();
		}
	}
	if (devintst & BV(DEV_STAT))
	{
		printf("GOT USB RESET\n");
	}
	if (devintst & BV(FRAME))
	{

	}
	if (devintst & BV(EP_FAST))
	{
		printf("Fast?!\n");
	}
	if (devintst & BV(TXENDPKT))
	{
		// Packet was received by USB controller
	}
	if (devintst & BV(ERR_INT))
	{
		printf("ERROR INTERRUPT ******************************\n");
	}
	//else
	//{
	//	printf("Got unknown interrupt: %x\n", devintst);
	//	for(int i = 0; i < 100000000; i++);
	//}
	LPC_USB->USBDevIntClr = devintst;
}

void usb_bulk_send(uint8_t *data, uint32_t len)
{
	bulk_data_to_send = data;
	bulk_num_to_send = len;
	bulk_num_sent = 0;
}

void usb_bulk_receive(uint8_t *data, uint32_t len)
{
	bulk_data_to_receive = data;
	bulk_num_to_receive = len;
	bulk_num_received = 0;
}

void usb_control_send(uint8_t *data, uint32_t len)
{
	control_data_to_send = data;
	control_num_to_send = len;
	control_num_sent = 0;
}

void usb_control_receive(uint8_t *data, uint32_t len)
{
	control_data_to_receive = data;
	control_num_to_receive = len;
	control_num_received = 0;
}

void usb_set_address(uint16_t address)
{
	usb_sie_command(CMD_SET_ADDRESS, BV(DEV_EN) | (address & 0x7f));
	usb_sie_command(CMD_SET_ADDRESS, BV(DEV_EN) | (address & 0x7f));
}

void usb_stall_endpoint(uint16_t endpoint)
{
	usb_sie_command(0x40 + usb_endpoint_to_phy(endpoint), BV(EP_STATUS_ST));
}

void usb_enable_endpoint(uint16_t endpoint)
{
	usb_sie_command(0x40 + endpoint, 0x0);
}

void usb_set_configured()
{
	usb_sie_command(CMD_CONFIGURE_DEVICE, 0x01);
}

void usb_interrupt()
{
	usb_poll();
}

void usb_configure_endpoint(uint8_t endpoint, uint8_t msps)
{
        uint8_t phyep = usb_endpoint_to_phy(endpoint);
#ifdef VERBOSE_DEBUG
        printf("Enabling physical endpoint %d\n", phyep);
#endif
        usb_realize_endpoint(phyep, msps);
        ///for(int i = 0; i < 1000000; i++);
        usb_enable_endpoint_interrupt(phyep);
        usb_enable_endpoint(phyep);
        usb_enable_endpoint(phyep);
}
