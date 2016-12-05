#ifndef _USB_IMPL_H_
#define _USB_IMPL_H_

#include <stdint.h>

typedef struct
{
	uint8_t request_type;
	uint8_t request;
	uint8_t lvalue;
	uint8_t hvalue;
	uint16_t index;
	uint16_t len;
} usb_control_request_t;

extern usb_control_request_t current_control_request;
extern uint16_t usb_device_address;
typedef struct
{
	uint8_t length;
	uint8_t type;
	uint16_t usb;
	uint8_t device_class;
	uint8_t device_sub_class;
	uint8_t device_protocol;
	uint8_t max_packet_size;
	uint16_t vendor;
	uint16_t product;
	uint16_t bcd;
	uint8_t imanufacturer;
	uint8_t iproduct;
	uint8_t iserial;
	uint8_t num_configurations;
} usb_descriptor_t;

typedef struct
{
	uint8_t length;
	uint8_t descriptor_type;
	uint8_t interface_number;
	uint8_t alternate_setting;
	uint8_t num_endpoints;
	uint8_t interface_class;
	uint8_t interface_subclass;
	uint8_t interface_protocol;
	uint8_t iinterface;
} interface_descriptor_t;

typedef struct
{
	uint8_t length;
	uint8_t descriptor_type;
	uint16_t total_length;
	uint8_t num_interfaces;
	uint8_t configuration_value;
	uint8_t iconfiguration;
	uint8_t attributes;
	uint8_t max_power;
	interface_descriptor_t interface_descriptor;
} configuration_descriptor_t;

#define GET_STATUS		0x00
#define CLEAR_FEATURE		0x01
#define SET_FEATURE		0x03
#define SET_ADDRESS		0x05
#define GET_DESCRIPTOR		0x06
#define SET_DESCRIPTOR		0x07
#define GET_CONFIGURATION	0x08
#define SET_CONFIGURATION	0x09

#define CONTROL_DEVICE_TO_HOST	0x80

void usb_control_request(uint8_t *data, uint32_t len);

#endif
