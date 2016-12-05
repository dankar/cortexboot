#include "inc/LPC17xx.h"
#include "gpio.h"
#include "common.h"
#include "uart.h"
#include "usb_impl.h"
#include "usb.h"

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

// Endpoint status bits
#define STP 		2

// Command clear buffer
#define CMD_CLEAR_BUFFER	0xF2
// Command validate data
#define CMD_VALIDATE_BUFFER	0xFA

// Set Mode command
#define CMD_SET_MODE 	0xF3
// Set mode bits
#define AP_CLK		0
#define INAK_CI		1
#define INAK_CO		2

// USBEpIntSt register bits

#define EP0RX		0
#define EP0TX		1


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

uint8_t *data_to_send = 0;
uint8_t num_to_send = 0;
uint8_t num_sent = 0;

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


uint32_t usb_init()
{
	set_bit32(&LPC_SC->PCONP, PCUSB); // Enable USB power

	LPC_SC->USBCLKCFG = 0x09; // pll0 = 480MHz, divisor 10 gives 48MHz

	LPC_USB->USBClkCtrl |= BV(DEV_CLK_EN) | BV(AHB_CLK_EN); // Enable clocks

	uart_print("USB clock status: ");
	while(!((LPC_USB->USBClkSt & (BV(DEV_CLK_EN)|BV(AHB_CLK_EN))) == (BV(DEV_CLK_EN)|BV(AHB_CLK_EN)))); // Wait for clock status OK

	uart_println("ok");

	// set_pin_debug(1);

	set_pin_function(0, 29, 0x01); // Set USB D-/D+ function
	set_pin_function(0, 30, 0x01);

	set_pin_function(1, 18, 0x01); // USB_UP_LED
	set_pin_function(1, 30, 0x02); // VBUS

	set_pin_mode(1, 30, NEITHER);
	set_pin_mode(0, 29, NEITHER);
	set_pin_mode(0, 30, NEITHER);


	LPC_USB->USBDevIntClr |= BV(EP_RLZED);

	uart_print("Current value of realized endpoints: ");
	uart_print_int(LPC_USB->USBReEp);
	uart_println("");

	LPC_USB->USBEpInd = 0x00;
	LPC_USB->USBMaxPSize = MAX_PACKET_SIZE0;

	uart_println("Waiting for realization of EP0...");

	while(!(LPC_USB->USBDevIntSt & BV(EP_RLZED)));

	LPC_USB->USBDevIntClr |= BV(EP_RLZED);

	LPC_USB->USBEpInd = 0x01;
	LPC_USB->USBMaxPSize = MAX_PACKET_SIZE0;

	uart_println("Waiting for realization of EP1..");

	while(!(LPC_USB->USBDevIntSt & BV(EP_RLZED)));

	LPC_USB->USBDevIntClr |= BV(EP_RLZED);

	uart_println("Control endpoints are now realized");

	LPC_USB->USBEpIntClr = 0xffffffff; // Clear all interrupts
	LPC_USB->USBDevIntClr = 0xffffffff;

	LPC_USB->USBEpIntEn |= BV(0) | BV(1); // enable interrupts from EP0TX and EP0RX

	uart_println("Enabled interrupts");

	usb_sie_command(CMD_SET_MODE, BV(AP_CLK) /*| BV(INAK_CI) | BV(INAK_CO)*/);

	uart_println("Set AP_CLK to 1");

	usb_sie_command(CMD_SET_ADDRESS, 0x00 |BV(DEV_EN));

	usb_sie_command(CMD_SET_STATUS, BV(CON));

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
	LPC_USB->USBCtrl = ((endpoint & 0xF) << 2) | BV(RD_EN);

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
	LPC_USB->USBCtrl = ((endpoint & 0x0F) << 2) | BV(WR_EN);

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
	usb_sie_command_ep_nd(endpoint+1, CMD_VALIDATE_BUFFER);
}

void usb_data_in_stage()
{
	uint32_t len = num_to_send - num_sent;

	if(len > MAX_PACKET_SIZE0)
		len = MAX_PACKET_SIZE0;

	usb_write_endpoint(0x00, data_to_send + num_sent, len);

	num_sent += len;

	if(num_sent == num_to_send)
	{
		data_to_send = 0;
		num_sent = 0;
		num_to_send = 0;
	}
}
#define SETUP_STAGE	0
#define IN_STAGE	1
#define OUT_STAGE	2

void usb_endpoint0(uint32_t stage)
{
	uint8_t control_request[MAX_PACKET_SIZE0];
	uint32_t num_recv = 0;

	for(int i = 0; i < MAX_PACKET_SIZE0; i++)
	{
		control_request[i] = 0;
	}


	switch(stage)
	{
	case SETUP_STAGE:
		uart_println("SETUP STAGE");
		data_to_send = 0; num_to_send = 0; num_sent = 0;
		num_recv = usb_read_endpoint(0x00, control_request);
		usb_control_request(control_request, num_recv);
		if(data_to_send)
		{
			uart_println("There is data to send, sending one packet...");
			usb_data_in_stage();
		}
		else
		{
			//uart_println("No data to send...");
		}
		break;
	case IN_STAGE:
		uart_println("IN STAGE");
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
		uart_println("OUT STAGE");
		if(current_control_request.request_type & CONTROL_DEVICE_TO_HOST)
		{
			uart_println("Get status");
			usb_read_endpoint(0x00, control_request); // get status
		}
		else
		{
			uart_println("Not implemented");
		}
	}
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
			LPC_USB->USBEpIntClr = BV(EP0RX); // Select endpoint and clear interrupt
			while((LPC_USB->USBDevIntSt & BV(CDFULL)) == 0);
			uint32_t status = LPC_USB->USBCmdData;

			if(status & BV(STP))
			{
				usb_endpoint0(SETUP_STAGE);
			}
			else
			{
				usb_endpoint0(OUT_STAGE);
			}
		}

		if(epint & BV(EP0TX))
		{
			LPC_USB->USBEpIntClr = BV(EP0TX);
			uart_println("Got TX int");
			usb_endpoint0(IN_STAGE);
		}
		LPC_USB->USBDevIntClr = BV(EP_SLOW);
	}
	else if (devintst & BV(DEV_STAT))
	{
		uart_println("GOT USB RESET");
		LPC_USB->USBDevIntClr = BV(DEV_STAT);
	}
	else if (devintst & BV(FRAME))
	{
	}
	else
	{
		uart_print("Got unknown interupt: ");
		uart_print_hex32(devintst);
		uart_println("");
		LPC_USB->USBDevIntClr = devintst;
	}
}

void usb_control_send(uint8_t *data, uint32_t len)
{

	uart_print("Set up data: ");
	uart_print_int(len);
	uart_print(" bytes total, ");
	uart_print_hex_str(data, len);
	uart_println("");
	uart_print_hex_str(data, len);
	uart_println("");

	data_to_send = data;
	num_to_send = len;
	num_sent = 0;
}

void usb_set_address(uint16_t address)
{
	usb_sie_command(CMD_SET_ADDRESS, BV(DEV_EN) | (address & 0x7f));
	usb_sie_command(CMD_SET_ADDRESS, BV(DEV_EN) | (address & 0x7f));
}

void usb_stall_endpoint(uint16_t endpoint)
{
	usb_sie_command(0x40 + endpoint, BV(EP_STATUS_ST));
}
