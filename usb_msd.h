#ifndef _USB_MSD_H_

#include <stdint.h>

extern uint8_t msd_has_status;
uint8_t usb_msd_has_more_data();
uint8_t usb_msd_write_active();
uint8_t usb_msd_wants_more_data(const uint8_t *data, uint32_t len);
void usb_msd_out(const uint8_t *data, uint32_t len);
void usb_msd_status();

#endif
