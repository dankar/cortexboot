#include "usb_device.h"
#include "usb.h"
#include "uart.h"
#include "common.h"
#include <wchar.h>
#include "descriptors.h"

uint16_t usb_device_address = 0;
usb_control_request_t current_control_request;

uint16_t current_idle = 0;
uint16_t current_configuration = 0;
uint16_t current_report = 0;
uint8_t string_buffer[255];
usb_string_t *usb_string = (usb_string_t*)string_buffer;

void print_control_struct(usb_control_request_t *req)
{
	printf("\tRequestType: %x\n", req->request_type);
	printf("\tRequest: %x\n", req->request);
	printf("\tlvalue: %x\n", req->lvalue);
	printf("\thvalue: %x\n", req->hvalue);
	printf("\tIndex: %x\n", req->index);
	printf("\tlen: %x\n", req->len);
}


void usb_send_string()
{
	uint32_t bytes_to_send;
	uint8_t string_index = current_control_request.lvalue - 1;
	uint8_t string_len = wstrlen(strings[string_index]);

	memset(string_buffer, '\0', 255);
	usb_string->length = sizeof(usb_string_t) + string_len * 2 - 1;
	usb_string->descriptor_type = STRING_DESCRIPTOR;
	memcpy(usb_string->string, strings[string_index], string_len * 2);
	bytes_to_send = MIN(usb_string->length, current_control_request.len);
	usb_control_send(string_buffer, bytes_to_send);
}

void usb_get_descriptor()
{
	uint32_t bytes_to_send;
	switch(current_control_request.hvalue)
        {
        case DEVICE_DESCRIPTOR:
#ifdef VERBOSE_DEBUG
		printf("Got request for device descriptor\n");
#endif
                bytes_to_send = MIN(device_descriptor.length, current_control_request.len);
                usb_control_send((uint8_t*)&device_descriptor, bytes_to_send);
                break;
        case CONFIG_DESCRIPTOR:
#ifdef VERBOSE_DEBUG
                printf("Got request for config descriptor\n");
#endif
                bytes_to_send = MIN(configuration_descriptor.config.total_length, current_control_request.len);
                usb_control_send((uint8_t*)&configuration_descriptor, bytes_to_send);
                break;
        case STRING_DESCRIPTOR:
#ifdef VERBOSE_DEBUG
                printf("Got request for string descriptor\n");
#endif
		if(current_control_request.lvalue == 0x00)
		{
			bytes_to_send = MIN(sizeof(string_descriptor), current_control_request.len);
			usb_control_send((uint8_t*)&string_descriptor, bytes_to_send);
		}
		else
		{
			usb_send_string();
		}
                break;
	case HID_REPORT_DESCRIPTOR:
#ifdef VERBOSE_DEBUG
		printf("Got request for report descriptor\n");
#endif
		bytes_to_send = MIN(sizeof(hid_report_descriptor), current_control_request.len);
		usb_control_send((uint8_t*)&hid_report_descriptor, bytes_to_send);
		break;
        default:
                // Unknown descriptor request, stall
#ifdef VERBOSE_DEBUG
		printf("!!!!!!!!!!!!!!!!!!!! Got request for unknown descriptor\n");
        	print_control_struct(&current_control_request);
                printf("Stalling endpoint\n");
#endif
	        usb_stall_endpoint(CONTROL_IN_ENDPOINT);
                break;
        }
}

void usb_unknown_request()
{
	printf("!!!!!!!!!!!!!!!!!!!!!!!!! Got unknown request\n");
        print_control_struct(&current_control_request);
}

uint8_t receive_buffer[1024];
uint8_t keyboard_buffer[8] = { 0, 0, 0, 0, 0, 0, 0, 0};

void usb_set_configuration(uint8_t value)
{
	if(current_configuration != value)
	{
		usb_configure_endpoint(INTERRUPT_ENDPOINT, 8);
		usb_configure_endpoint(BULK_IN_ENDPOINT, 64);
		usb_configure_endpoint(BULK_OUT_ENDPOINT, 64);
		current_configuration = value;
	}
	usb_write_endpoint(CONTROL_IN_ENDPOINT, 0, 0);
	usb_set_configured();

	usb_write_endpoint(INTERRUPT_ENDPOINT, keyboard_buffer, 8);
}

void usb_set_feature()
{
	print_control_struct(&current_control_request);
}

void usb_endpoint_interrupt()
{
	usb_write_endpoint(INTERRUPT_ENDPOINT, keyboard_buffer, 8);
}


void usb_control_request(uint8_t *data, uint32_t len, uint8_t *extra_data, uint32_t extra_len)
{
	uint32_t bytes_to_send;
	if(data)
	{
		current_control_request = *(usb_control_request_t*)data;
	}
	//fix_control_struct(req);

	//uart_println("Got usb control request: ");

	if(current_control_request.request_type == 0x80)
	{
		if(current_control_request.request == GET_DESCRIPTOR)
		{
			// get descriptor
			usb_get_descriptor();
		}
	}
	else if(current_control_request.request_type == 0x81)
	{
		if(current_control_request.request == GET_DESCRIPTOR)
		{
			usb_get_descriptor();
		}
	}
	else if(current_control_request.request_type == 0x00)
	{

		switch(current_control_request.request)
		{
		case SET_ADDRESS:
			usb_device_address = current_control_request.lvalue;
			usb_write_endpoint(CONTROL_IN_ENDPOINT, 0, 0);
			break;
		case SET_CONFIGURATION:
#ifdef VERBOSE_DEBUG
			printf("Got set config\n");
#endif
			usb_set_configuration(current_control_request.lvalue);
			break;
		case SET_FEATURE:
#ifdef VERBOSE_DEBUG
			printf("Got set feature\n");
#endif
			usb_set_feature();
			break;
		default:
			usb_unknown_request();
			usb_stall_endpoint(CONTROL_IN_ENDPOINT);
		}
	}
	else if(current_control_request.request_type == 0x21)
	{
		switch(current_control_request.request)
		{
		case SET_IDLE:
			current_idle = current_control_request.lvalue | (current_control_request.hvalue << 8);
			usb_write_endpoint(CONTROL_IN_ENDPOINT, 0, 0);
			break;
		case SET_REPORT:
			if(extra_data)
			{
				current_report = extra_data[0];
				usb_write_endpoint(CONTROL_IN_ENDPOINT, 0, 0);
			}
			else
			{
				usb_control_receive(receive_buffer, current_control_request.len);
			}
			break;
		default:
			usb_unknown_request();
			usb_stall_endpoint(CONTROL_IN_ENDPOINT);
		}
	}
	else if(current_control_request.request_type == 0x02)
	{
		switch(current_control_request.request)
		{
		case GET_REPORT:
			bytes_to_send = MIN(8, current_control_request.len);
			usb_control_send(keyboard_buffer, bytes_to_send);
			break;
		default:
			usb_unknown_request();
			usb_stall_endpoint(CONTROL_IN_ENDPOINT);
		}
	}
	else if(current_control_request.request_type == 0xa1)
	{
		switch(current_control_request.request)
		{
		case GET_MAX_LUN:
			usb_write_endpoint(CONTROL_IN_ENDPOINT, &max_lun, 1);
			break;
		default:
			usb_unknown_request();
			usb_stall_endpoint(CONTROL_IN_ENDPOINT);
		}
	}
	else
	{
		usb_unknown_request();
		usb_stall_endpoint(CONTROL_IN_ENDPOINT);
	}
}
