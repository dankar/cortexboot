#ifndef _USB_H_
#define _USB_H_

#include <stdint.h>

#define MAX_PACKET_SIZE0 8

void usb_control_send(uint8_t *data, uint32_t len);

void usb_stall_endpoint(uint16_t endpoint);
void usb_write_endpoint(uint32_t endpoint, uint8_t *data, uint32_t count);
uint32_t usb_read_endpoint(uint32_t endpoint, uint8_t *data);
void usb_set_address(uint16_t address);
void usb_poll();
uint32_t usb_init();

#endif