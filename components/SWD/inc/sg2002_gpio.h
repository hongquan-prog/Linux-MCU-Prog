#pragma once

#include "compiler.h"
#include <stdbool.h>

typedef struct
{
    __IO uint32_t DR;
    __IO uint32_t DDR;
    uint32_t RESERVED_0[10];
    __IO uint32_t INTEN;
    __IO uint32_t INTMASK;
    __IO uint32_t INTTYPE_LEVEL;
    __IO uint32_t INT_POLARITY;
    __IO uint32_t INTSTATUS;
    __IO uint32_t RAW_INTSTATUS;
    __IO uint32_t DEBOUNCE;
    __IO uint32_t EOI;
    __IO uint32_t EXT_PORT;
    uint32_t RESERVED_1[3];
    __IO uint32_t LS_SYNC;
} SG2002_GPIO_Type;

#if defined(__cplusplus)
extern "C" {
#endif

SG2002_GPIO_Type *SG2002_GPIOBase(int fd, int pin);
void SG2002_Clean(void);

static inline void SG2002_PinInit(volatile void *base, uint32_t pin, const bool output)
{
    volatile SG2002_GPIO_Type *port = (volatile SG2002_GPIO_Type *)base;

    if (output)
    {
        port->DDR |= (0x1UL << pin);
    }
    else
    {
        port->DDR &= ~(0x1UL << pin);
    }
}

static inline void SG2002_WritePinOutput(volatile void *base, uint32_t pin, uint8_t output)
{
    if (output == 0U)
    {
        ((volatile SG2002_GPIO_Type *)base)->DR &= ~(0x1UL << pin);
    }
    else
    {
        ((volatile SG2002_GPIO_Type *)base)->DR |= 1UL << pin;
    }
}

static inline void SG2002_SetPinsOutput(volatile void *base, uint32_t pin)
{
    ((volatile SG2002_GPIO_Type *)base)->DR |= (1 << pin);
}

static inline void SG2002_ClearPinsOutput(volatile void *base, uint32_t pin)
{
    ((volatile SG2002_GPIO_Type *)base)->DR &= ~(1UL << pin);
}

static inline uint32_t SG2002_ReadPinInput(volatile void *base, uint32_t pin)
{
    return ((((volatile SG2002_GPIO_Type *)base)->EXT_PORT >> pin) & 0x01U);
}

#if defined(__cplusplus)
}
#endif