#include "usb_impl.h"
#include "usb.h"
#include "uart.h"
#include "common.h"

uint16_t usb_device_address = 0;
usb_control_request_t current_control_request;

void print_control_struct(usb_control_request_t *req)
{
	uart_print("Request Type: ");
	uart_print_hex(req->request_type);
	uart_println("");
	uart_print("Request: ");
	uart_print_hex(req->request);
	uart_println("");
	uart_print("lValue: ");
	uart_print_hex(req->lvalue);
	uart_print("hValue: ");
	uart_print_hex(req->hvalue);
	uart_println("");
	uart_print("Index: ");
	uart_print_hex16(req->index);
	uart_println("");
	uart_print("Length: ");
	uart_print_hex16(req->len);
	uart_println("");
}

usb_descriptor_t device_descriptor = { 18,
					0x01,
					0x200,
					0x00,
					0x00,
					0x00,
					0x8,
					0xefef,
					0xcdcd,
					0x00,
					0x00,
					0x00,
					0x00,
					0x01 };

configuration_descriptor_t configuration_descriptor = { 9,
				0x02,
				9+9,
				1,
				0x5,
				0,
				0x80,
				250,
				{	0x09,
					0x04,
					0x00,
					0x00,
					0,
					0x00,
					0x00,
					0x00,
					0 } };

void usb_control_request(uint8_t *data, uint32_t len)
{
	current_control_request = *(usb_control_request_t*)data;
	//fix_control_struct(req);

	//uart_println("Got usb control request: ");
	print_control_struct(&current_control_request);

	if(current_control_request.request_type == 0x80 && current_control_request.request == 0x06)
	{
		// get descriptor
		if(current_control_request.hvalue == 0x01 && current_control_request.lvalue == 0x00)
		{
			uart_println("Got request for device descriptor");
			uint32_t bytes_to_send = MIN(device_descriptor.length, current_control_request.len);
			usb_control_send((uint8_t*)&device_descriptor, bytes_to_send);
		}
		else if(current_control_request.hvalue == 0x02 && current_control_request.lvalue == 0x00)
		{
			uart_println("Got request for config descriptor");
			uint32_t bytes_to_send = MIN(configuration_descriptor.total_length, current_control_request.len);
			usb_control_send((uint8_t*)&configuration_descriptor, bytes_to_send);
		}
		else
		{
			// Unknown descriptor request, stall
			uart_println("Stalling endpoint");
			usb_stall_endpoint(0x00);
		}
	}
	else if(current_control_request.request_type == 0x00 && current_control_request.request == 0x05)
	{
		uart_print("Got set address: ");
		uart_print_hex32(current_control_request.lvalue);
		uart_println("");

		usb_device_address = current_control_request.lvalue;

		usb_write_endpoint(0x00, 0, 0);
	}
}
