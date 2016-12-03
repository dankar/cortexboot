#include "common.h"

uint8_t check_bit8(const volatile uint8_t *reg, uint8_t bit);
void set_bit8(volatile uint8_t *reg, uint8_t bit);
void clear_bit8(volatile uint8_t *reg, uint8_t bit);
uint8_t check_bit16(const volatile uint16_t *reg, uint8_t bit);
void set_bit16(volatile uint16_t *reg, uint8_t bit);
void clear_bit16(volatile uint16_t *reg, uint8_t bit);
uint8_t check_bit32(const volatile uint32_t *reg, uint8_t bit);
void set_bit32(volatile uint32_t *reg, uint8_t bit);
void clear_bit32(volatile uint32_t *reg, uint8_t bit);

uint32_t strlen(const char *str)
{
        uint32_t count = 0;
        while(*str)
        {
                count++;
                str++;
        }

        return count;
}

