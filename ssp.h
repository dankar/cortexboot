#ifndef _SSP_H_
#define _SSP_H_

#include <stdint.h>

void ssp_init_spi();
void spi_send_data(uint8_t *data, uint32_t len);
void spi_send_receive(uint8_t *data, uint32_t len);
uint8_t spi_send(uint8_t data);

#endif
