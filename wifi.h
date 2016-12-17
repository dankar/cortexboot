#ifndef _WIFI_H_
#define _WIFI_H_

#include <stdint.h>

void wifi_interrupt();
void wifi_init();
void wifi_select(uint8_t select);
void wifi_disable_irq();
void wifi_enable_irq();
uint8_t wifi_connect();

uint8_t wifi_send(const uint8_t *data, uint32_t len);
uint8_t wifi_recv(uint8_t *data, uint32_t len);

#endif
