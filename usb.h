#ifndef _USB_H_
#define _USB_H_

#include <stdint.h>

#define MAX_PACKET_SIZE0 8

void usb_control_send(uint8_t *data, uint32_t len);
void usb_control_receive(uint8_t *data, uint32_t len);
void usb_bulk_data_in();
void usb_realize_endpoint(uint32_t endpoint, uint32_t max_packet_size);
void usb_enable_endpoint_interrupt(uint32_t endpoint);
void usb_stall_endpoint(uint16_t endpoint);
void usb_write_endpoint(uint32_t endpoint, uint8_t *data, uint32_t count);
uint32_t usb_read_endpoint(uint32_t endpoint, uint8_t *data);
void usb_set_address(uint16_t address);
void usb_set_configured();
void usb_enable_endpoint(uint16_t endpoint);
void usb_poll();
uint32_t usb_init();
uint8_t usb_endpoint_to_phy(uint8_t endpoint);
void usb_interrupt();
void usb_configure_endpoint(uint8_t endpoint, uint8_t msps);
void usb_bulk_send(uint8_t *data, uint32_t len);
void usb_bulk_receive(uint8_t *data, uint32_t len);

#endif
