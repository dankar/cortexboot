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
uint16_t bs16(uint16_t w);

void delay(uint32_t cyc)
{
	for(int i = 0; i < cyc; i++);
}

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
uint32_t wstrlen(const wchar_t *str)
{
        uint32_t count = 0;
        while(*str)
        {
                count++;
                str++;
        }

        return count;
}

void *memcpy(void *dst, const void* src, size_t num)
{
	uint8_t *d = dst;
	const uint8_t *s = src;
	for(int i = 0; i < num; i++)
	{
		d[i] = s[i];
	}

	return dst;
}

void *memset(void *dst, int val, size_t num)
{
	uint8_t *d = dst;
	for(int i = 0; i < num; i++)
	{
		d[i] = val;
	}

	return dst;
}

void sleep()
{
	for(int i = 0; i < 100000; i++);
}

uint8_t is_upper(uint8_t ascii)
{
        return ascii >= 'A' && ascii <= 'Z';
}

uint8_t to_lower(uint8_t ascii)
{
        if(is_upper(ascii))
                return ascii += 'a' - 'A';
        else
                return ascii;
}

uint8_t is_alpha(uint8_t ascii)
{
        if(to_lower(ascii) >= 'a' && to_lower(ascii) <= 'z')
                return 1;
        else
                return 0;
}

uint8_t is_numeric(uint8_t ascii)
{
        if(ascii >= '0' && ascii <= '9')
                return 1;
        else
                return 0;
}

