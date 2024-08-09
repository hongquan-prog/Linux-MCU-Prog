#pragma once

#include "compiler.h"
#include <stdbool.h>

typedef struct
{
    __IO uint32_t GPFSEL0;
    __IO uint32_t GPFSEL1;
    __IO uint32_t GPFSEL2;
    __IO uint32_t GPFSEL3;
    __IO uint32_t GPFSEL4;
    __IO uint32_t GPFSEL5;
    __IO uint32_t RESERVED0;
    __IO uint32_t GPSET0;
    __IO uint32_t GPSET1;
    __IO uint32_t RESERVED1;
    __IO uint32_t GPCLR0;
    __IO uint32_t GPCLR1;

    __IO uint32_t RESERVED2;
    __IO uint32_t GPLEV0;
    __IO uint32_t GPLEV1;

    __IO uint32_t RESERVED3;
    __IO uint32_t GPEDS0;
    __IO uint32_t GPEDS1;

    __IO uint32_t RESERVED4;
    __IO uint32_t GPREN0;
    __IO uint32_t GPREN1;

    __IO uint32_t RESERVED5;
    __IO uint32_t GPFEN0;
    __IO uint32_t GPFEN1;

    __IO uint32_t RESERVED6;
    __IO uint32_t GPHEN0;
    __IO uint32_t GPHEN1;

    __IO uint32_t RESERVED7;
    __IO uint32_t GPLEN0;
    __IO uint32_t GPLEN1;

    __IO uint32_t RESERVED8;
    __IO uint32_t GPAREN0;
    __IO uint32_t GPAREN1;

    __IO uint32_t RESERVED9;
    __IO uint32_t GPAFEN0;
    __IO uint32_t GPAFEN1;

    __IO uint32_t RESERVED10;
    __IO uint32_t GPPUD;
    __IO uint32_t GPPUDCLK0;
    __IO uint32_t GPPUDCLK1;
} BCM2835_GPIO;

#if defined(__cplusplus)
extern "C" {
#endif

BCM2835_GPIO *BCM2835_GPIOBase(int fd, int pin);
void BCM2835_Clean(void);

static inline void BCM2835_PinInit(volatile void *base, uint32_t pin, const bool output)
{
    int offset = (pin % 10) * 3;
    volatile uint32_t *reg = (volatile uint32_t *)base + pin / 10;

    *reg &= ~(0x7 << offset);
    if (output)
    {
        *reg |= (0x1 << offset);
    }
}

static inline void BCM2835_WritePinOutput(volatile void *base, uint32_t pin, uint8_t output)
{
    int offset = pin % 32;
    volatile uint32_t *reg = NULL;

    if (output == 0U)
    {
        reg = &(((volatile BCM2835_GPIO *)base)->GPCLR0) + (pin / 32);
        *reg = 1 << offset;
    }
    else
    {
        reg = &(((volatile BCM2835_GPIO *)base)->GPSET0) + (pin / 32);
        *reg = 1 << offset;
    }
    __sync_synchronize();
}

static inline void BCM2835_SetPinsOutput(volatile void *base, uint32_t pin)
{
    volatile uint32_t *reg = &(((volatile BCM2835_GPIO *)base)->GPSET0) + (pin / 32);
    *reg = 1 << (pin % 32);
    __sync_synchronize();
}

static inline void BCM2835_ClearPinsOutput(volatile void *base, uint32_t pin)
{
    volatile uint32_t *reg = &(((volatile BCM2835_GPIO *)base)->GPCLR0) + (pin / 32);
    *reg = 1 << (pin % 32);
    __sync_synchronize();
}

static inline uint32_t BCM2835_ReadPinInput(volatile void *base, uint32_t pin)
{
    volatile uint32_t *reg = &(((volatile BCM2835_GPIO *)base)->GPLEV0) + (pin / 32);
    return (*reg >> (pin % 32)) & 0x1U;
}

#if defined(__cplusplus)
}
#endif