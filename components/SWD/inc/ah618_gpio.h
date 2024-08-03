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

typedef enum
{
    AH618_PULL_NONE = 0U,
    AH618_PULL_UP = 1U,
    AH618_PULL_DOWN = 2U
} ah618_pull_t;

typedef enum
{
    AH618_DRV_0,
    AH618_DRV_1,
    AH618_DRV_2,
    AH618_DRV_3
} ah618_drv_t;

enum AH618_GPIO
{
    AH618_GPIOC = 2,
    AH618_GPIOF = 5,
    AH618_GPIOG = 6,
    AH618_GPIOH = 7,
    AH618_GPIOI = 9,
    AH618_GPIOL = 0
};


#if defined(__cplusplus)
extern "C" {
#endif

AH618_GPIO_Type *AH618_GPIOBase(int fd, enum AH618_GPIO gpio);

void AH618_Clean(void);

static inline void AH618_PinInit(volatile AH618_GPIO_Type *base, uint32_t pin, const bool output)
{
    int offset = (pin % 8) * 4;
    volatile uint32_t *reg = (volatile uint32_t *)base + pin / 8;

    *reg &= ~(0x7 << offset);
    if (output)
    {
        *reg |= (0x1 << offset);
    }
}

static inline void AH618_PullInit(volatile AH618_GPIO_Type *base, uint32_t pin, ah618_pull_t pull)
{
    int offset = (pin % 16) * 4;
    volatile uint32_t *reg = ((volatile uint32_t *)(&base->PUL0)) + pin / 16;

    *reg &= ~(0x3 << offset);
    *reg |= (pull << offset);
}

static inline void AH618_DrvInit(volatile AH618_GPIO_Type *base, uint32_t pin, ah618_drv_t drv)
{
    int offset = (pin % 16) * 4;
    volatile uint32_t *reg = ((volatile uint32_t *)(&base->DRV0)) + pin / 16;

    *reg &= ~(0x3 << offset);
    *reg |= (drv << offset);
}

static inline void AH618_WritePinOutput(volatile AH618_GPIO_Type *base, uint32_t pin, uint8_t output)
{
    if (output == 0U)
    {
        base->DATA &= ~(1UL << pin);
    }
    else
    {
        base->DATA |= 1UL << pin;
    }
}

static inline void AH618_SetPinsOutput(volatile AH618_GPIO_Type *base, uint32_t pin)
{
    base->DATA |= (1 << pin);
}

static inline void AH618_ClearPinsOutput(volatile AH618_GPIO_Type *base, uint32_t pin)
{
    base->DATA &= ~(1UL << pin);
}

static inline void AH618_TogglePinsOutput(volatile AH618_GPIO_Type *base, uint32_t pin)
{
    if (base->DATA & (1 << pin))
    {
        base->DATA &= ~(1UL << pin);
    }
    else
    {
        base->DATA |= (1 << pin);
    }
}

static inline uint32_t AH618_ReadPinInput(volatile AH618_GPIO_Type *base, uint32_t pin)
{
    return (((base->DATA) >> pin) & 0x01U);
}


#if defined(__cplusplus)
}
#endif