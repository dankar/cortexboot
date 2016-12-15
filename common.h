#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>
#include <stddef.h>
#include <wchar.h>

#define BV(b) (1 << b)

#define MAX(x, y) ((x > y) ? x : y)
#define MIN(x, y) ((x > y) ? y : x)

uint32_t strlen(const char *str);
uint32_t wstrlen(const wchar_t *str);
void *memcpy(void *dst, const void* src, size_t num);
void *memset(void *dst, int val, size_t num);
uint8_t is_upper(uint8_t ascii);
uint8_t to_lower(uint8_t ascii);
uint8_t is_alpha(uint8_t ascii);
uint8_t is_numeric(uint8_t ascii);

void sleep();

void delay(uint32_t cyc);

inline uint16_t bs16(uint16_t w)
{
	return (w >> 8) | ((w & 0xff) << 8);
}

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
