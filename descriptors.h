#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

#include <stdint.h>
#include "usb_proto.h"

typedef struct
{
	usb_configuration_descriptor_t config;
	usb_interface_descriptor_t interface;
	usb_hid_descriptor_t hid_keyboard;
	usb_endpoint_descriptor_t interrupt_in;
} PACK usb_descriptors_t ;

extern const wchar_t *strings[];
extern usb_device_descriptor_t device_descriptor;
extern usb_descriptors_t configuration_descriptor;
extern usb_string_descriptor_t string_descriptor;
extern usb_keyboard_report_descriptor_t hid_report_descriptor;

#endif
