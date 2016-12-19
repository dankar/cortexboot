#ifndef _USB_IMPL_H_
#define _USB_IMPL_H_

#include <stdint.h>
#include "usb_proto.h"

extern uint16_t usb_device_address;
extern usb_control_request_t current_control_request;
extern uint8_t keyboard_buffer[8];

void usb_control_request(uint8_t *data, uint32_t len, uint8_t* extra_data, uint32_t extra_len);
void usb_endpoint_interrupt();

#endif
