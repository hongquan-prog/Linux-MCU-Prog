#pragma once

#include "compiler.h"
#include <stdbool.h>

typedef struct
{
    __IO uint32_t CFG0;
    __IO uint32_t CFG1;
    __IO uint32_t CFG2;
    __IO uint32_t CFG3;
    __IO uint32_t DATA;
    __IO uint32_t DRV0;
    __IO uint32_t DRV1;
    __IO uint32_t PUL0;
    __IO uint32_t PUL1;
} AH618_GPIO_Type;

#if defined(__cplusplus)
extern "C" {
#endif

AH618_GPIO_Type *AH618_GPIOBase(int fd, int pin);
void AH618_Clean(void);

static inline void AH618_PinInit(volatile void *base, uint32_t pin, const bool output)
{
    int offset = (pin % 8) * 4;
    volatile uint32_t *reg = (volatile uint32_t *)base + pin / 8;

    *reg &= ~(0x7 << offset);
    if (output)
    {
        *reg |= (0x1 << offset);
    }
}

static inline void AH618_WritePinOutput(volatile void *base, uint32_t pin, uint8_t output)
{
    if (output == 0U)
    {
        ((volatile AH618_GPIO_Type *)base)->DATA &= ~(1UL << pin);
    }
    else
    {
        ((volatile AH618_GPIO_Type *)base)->DATA |= 1UL << pin;
    }
}

static inline void AH618_SetPinsOutput(volatile void *base, uint32_t pin)
{
    ((volatile AH618_GPIO_Type *)base)->DATA |= (1 << pin);
}

static inline void AH618_ClearPinsOutput(volatile void *base, uint32_t pin)
{
    ((volatile AH618_GPIO_Type *)base)->DATA &= ~(1UL << pin);
}

static inline void AH618_TogglePinsOutput(volatile void *base, uint32_t pin)
{
    if (((volatile AH618_GPIO_Type *)base)->DATA & (1 << pin))
    {
        ((volatile AH618_GPIO_Type *)base)->DATA &= ~(1UL << pin);
    }
    else
    {
        ((volatile AH618_GPIO_Type *)base)->DATA |= (1 << pin);
    }
}

static inline uint32_t AH618_ReadPinInput(volatile void *base, uint32_t pin)
{
    return ((((volatile AH618_GPIO_Type *)base)->DATA >> pin) & 0x01U);
}

#if defined(__cplusplus)
}
#endif