#ifndef _USB_PROTO_H_
#define _USB_PROTO_H_

#include <stdint.h>

#define SET_ADDRESS             0x05
#define GET_DESCRIPTOR          0x06

#define DEVICE_DESCRIPTOR               0x01
#define CONFIG_DESCRIPTOR               0x02
#define STRING_DESCRIPTOR               0x03
#define INTERFACE_DESCRIPTOR            0x04
#define DEVICE_QUALIFIER_DESCRIPTOR     0x06
#define HID_DESCRIPTOR                  0x21
#define HID_REPORT_DESCRIPTOR           0x22

#define ENDPOINT_DIRECTION              BV(7)
#define ENDPOINT_OUT                    0
#define ENDPOINT_IN                     1

#define ENDPOINT_CONTROL                0x00
#define ENDPOINT_ISOCHRONOUS            0x01
#define ENDPOINT_BULK                   0x02
#define ENDPOINT_INTERRUPT              0x03

#define GET_REPORT                      0x01
#define GET_IDLE                        0x02
#define GET_PROTOCOL                    0x03
#define SET_REPORT                      0x09
#define SET_IDLE                        0x0a
#define SET_PROTOCOL                    0x0b

#define LANG_US                 0x0409

#define CONTROL_ENDPOINT 0x00
#define INTERRUPT_ENDPOINT 0x81

#define GET_STATUS              0x00
#define CLEAR_FEATURE           0x01
#define SET_FEATURE             0x03
#define SET_ADDRESS             0x05
#define GET_DESCRIPTOR          0x06
#define SET_DESCRIPTOR          0x07
#define GET_CONFIGURATION       0x08
#define SET_CONFIGURATION       0x09

#define CONTROL_DEVICE_TO_HOST  0x80

#define PACK __attribute__ ((packed))
#define BCD(Major, Minor, Revision) (((Major & 0xFF) << 8) | ((Minor & 0x0F) << 4) | (Revision & 0x0F))

typedef struct
{
	uint8_t request_type;
	uint8_t request;
	uint8_t lvalue;
	uint8_t hvalue;
	uint16_t index;
	uint16_t len;
} PACK usb_control_request_t;

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
} PACK usb_device_descriptor_t;

typedef struct
{
	uint8_t length;
	uint8_t descriptor_type;
	uint8_t endpoint_address;
	uint8_t attributes;
	uint16_t max_packet_size;
	uint8_t interval;
} PACK usb_endpoint_descriptor_t;

typedef struct
{
	uint8_t length;
	uint8_t descriptor_type;
	uint16_t bcd_hid;
	uint8_t country;
	uint8_t num_descriptors;
	uint8_t report_type;
	uint16_t report_descriptor_length;
} PACK usb_hid_descriptor_t;

typedef struct
{
	uint8_t length;
	uint8_t descriptor_type;
	uint8_t interface_number;
	uint8_t alternate_setting;
	uint8_t num_endpoints;
	uint8_t interface_class;
	uint8_t interface_sub_class;
	uint8_t interface_protocol;
	uint8_t iinterface;
} PACK usb_interface_descriptor_t;

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
} PACK usb_configuration_descriptor_t;


typedef struct
{
	uint8_t data[65];
} PACK usb_keyboard_report_descriptor_t;

typedef struct
{
	uint8_t length;
	uint8_t descriptor_type;
	uint16_t lang_id;
} PACK usb_string_descriptor_t;

typedef struct
{
	uint8_t length;
	uint8_t descriptor_type;
	uint8_t string[1];
} PACK usb_string_t;

#endif
