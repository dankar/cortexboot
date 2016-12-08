#include "usb_impl.h"
#include "common.h"
#include <wchar.h>
#include "descriptors.h"

const wchar_t *strings[] = {
			L"dk Industries",
			L"USB stick",
			L"0000",
			L"USB Stick 2",
			L"Default config"
};

usb_device_descriptor_t device_descriptor = {
		.length = sizeof(usb_device_descriptor_t),
		.type = DEVICE_DESCRIPTOR,
		.usb = 0x200,
		.device_class = 0x00,
		.device_sub_class = 0x00,
		.device_protocol = 0x00,
		.max_packet_size = 0x8,
		.vendor = 0xefef,
		.product = 0xcdcd,
		.bcd = BCD(1, 1, 1),
		.imanufacturer = 0x01, // String 1
		.iproduct = 0x02, // String 2
		.iserial = 0x03, // String 3
		.num_configurations = 0x01
};

usb_descriptors_t configuration_descriptor = {
		.config = {
			.length = sizeof(usb_configuration_descriptor_t),
			.descriptor_type = CONFIG_DESCRIPTOR,
			.total_length = sizeof(usb_descriptors_t), //(config + interface + endpoint)
			.num_interfaces = 1,
			.configuration_value = 0x5,
			.iconfiguration = 0x04, // String 4
			.attributes = 0x80,
			.max_power = 250
		},
		.interface = {
			.length = sizeof(usb_interface_descriptor_t),
			.descriptor_type = INTERFACE_DESCRIPTOR,
			.interface_number = 0x00,
			.alternate_setting = 0x00,
			.num_endpoints = 1, // Num endpoints
			.interface_class = 0x03, // Class HID
			.interface_sub_class = 0x01,
			.interface_protocol = 0x01,
			.iinterface = 0x04, // String 5
		},
		.hid_keyboard = {
			.length = sizeof(usb_hid_descriptor_t),
			.descriptor_type = HID_DESCRIPTOR,
			.bcd_hid = BCD(1, 1, 1),
			.country = 0x00,
			.num_descriptors = 1,
			.report_type = HID_REPORT_DESCRIPTOR,
			.report_descriptor_length = sizeof(usb_keyboard_report_descriptor_t)
		},
		.interrupt_in = {
				.length = sizeof(usb_endpoint_descriptor_t),
				.descriptor_type = 0x05,
				.endpoint_address = INTERRUPT_ENDPOINT, // address
				.attributes = ENDPOINT_INTERRUPT,
				.max_packet_size = 8, // MPS
				.interval = 10 // interval
		}
};

usb_string_descriptor_t string_descriptor = { 4,
					  STRING_DESCRIPTOR,
					  LANG_US };
usb_keyboard_report_descriptor_t hid_report_descriptor = {
			{
			 0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07,
			 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01,
			 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01,
			 0x75, 0x08, 0x81, 0x01, 0x95, 0x05, 0x75, 0x01,
			 0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02,
			 0x95, 0x01, 0x75, 0x03, 0x91, 0x01, 0x95, 0x06,
			 0x75, 0x08, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x05,
			 0x07, 0x19, 0x00, 0x2A, 0xFF, 0x00, 0x81, 0x00,
			 0xC0
			}
};

