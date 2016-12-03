#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

#define BV(b) (1 << b)

uint32_t strlen(const char *str);

inline uint8_t check_bit8(const volatile uint8_t *reg, uint8_t bit)
{
	return ((*reg & BV(bit)) != 0) ? 1 : 0;
}

inline void set_bit8(volatile uint8_t *reg, uint8_t bit)
{
	*reg |= BV(bit);
}

inline void clear_bit8(volatile uint8_t *reg, uint8_t bit)
{
	*reg &= ~BV(bit);
}

inline uint8_t check_bit16(const volatile uint16_t *reg, uint8_t bit)
{
        return (*reg & BV(bit) != 0) ? 1 : 0;
}

inline void set_bit16(volatile uint16_t *reg, uint8_t bit)
{
        *reg |= BV(bit);
}

inline void clear_bit16(volatile uint16_t *reg, uint8_t bit)
{
        *reg &= ~BV(bit);
}

inline uint8_t check_bit32(const volatile uint32_t *reg, uint8_t bit)
{
        return (*reg & BV(bit) != 0) ? 1 : 0;
}

inline void set_bit32(volatile uint32_t *reg, uint8_t bit)
{
        *reg |= BV(bit);
}

inline void clear_bit32(volatile uint32_t *reg, uint8_t bit)
{
        *reg &= ~BV(bit);
}

#endif
