#pragma once

#include "compiler.h"
#include <stdbool.h>

typedef struct
{
    __IO uint32_t DATA_LOW;
    __IO uint32_t DATA_HIGH;
    __IO uint32_t DIR_LOW;
    __IO uint32_t DIR_HIGH;
    __IO uint32_t INT_EN_LOW;
    __IO uint32_t INT_EN_HIGH;
    __IO uint32_t INT_MASK_LOW;
    __IO uint32_t INT_MASK_HIGH;
    __IO uint32_t INT_TYPE_LOW;
    __IO uint32_t INT_TYPE_HIGH;
    __IO uint32_t INT_PARITY_LOW;
    __IO uint32_t INT_PARITY_HIGH;
    __IO uint32_t INT_BOTHEDGE_LOW;
    __IO uint32_t INT_BOTHEDGE_HIGH;
    __IO uint32_t DEBOUNCE_LOW;
    __IO uint32_t DEBOUNCE_HIGH;
    __IO uint32_t DBCLK_DIR_LOW;
    __IO uint32_t DBCLK_DIR_HIGH;
    __IO uint32_t DBCLK_DIV_CTRL;
    uint32_t RESERVED_0;
    __IO uint32_t INT_STATUS;
    uint32_t RESERVED_1;
    __IO uint32_t INT_RAW_STATUS;
    uint32_t RESERVED_2;
    __IO uint32_t INT_EOI_LOW;
    __IO uint32_t INT_EOI_HIGH;
    uint32_t RESERVED_3[2];
    __IO uint32_t EXT_PORT;
    uint32_t RESERVED_4;
    __IO uint32_t VENDOR_ID;
     uint32_t RESERVED_5[33];
    __IO uint32_t GROUP_CTRL_LOW;
    __IO uint32_t GROUP_CTRL_HIGH;
    __IO uint32_t VIRTUAL_EN;
} RK3588_GPIO_Type;

typedef struct
{
    __IO uint32_t GPIO0A_IOMUX_LOW;
    __IO uint32_t GPIO0A_IOMUX_HIGH;
    __IO uint32_t GPIO0B_IOMUX_LOW;
    uint32_t RESERVED_1;
    __IO uint32_t GPIO0A_DRV_LOW;
    __IO uint32_t GPIO0A_DRV_HIGH;
    __IO uint32_t GPIO0B_DRV_LOW;
    uint32_t RESERVED_2;
    __IO uint32_t GPIO0A_PULL;
    __IO uint32_t GPIO0B_PULL;
    __IO uint32_t GPIO0A_INT_EN;
    __IO uint32_t GPIO0B_INT_EN;
    __IO uint32_t GPIO0A_SMT;
    __IO uint32_t GPIO0B_SMT;
    __IO uint32_t GPIO0A_PDIS;
    __IO uint32_t GPIO0B_PDIS;
    __IO uint32_t XIN_CON;
} RK3588_PMU_Type;

enum RK3588_GPIO
{
    RK3588_GPIO0,
    RK3588_GPIO1,
    RK3588_GPIO2,
    RK3588_GPIO3,
    RK3588_GPIO4
};

typedef enum
{
    RK3588_PULL_NONE = 0U,
    RK3588_PULL_UP = 1U,
    RK3588_PULL_DOWN = 2U
} rk3588_pull_t;

#define BIT(nr)			((1) << (nr))

#if defined(__cplusplus)
extern "C" {
#endif

RK3588_GPIO_Type *RK3588_GPIOBase(int fd, enum RK3588_GPIO gpio);

void RK3588_Clean(void);

static inline void RK3588_PinInit(volatile RK3588_GPIO_Type *base, uint32_t pin, const bool output)
{
    volatile uint32_t *dir = (volatile uint32_t *)(&base->DIR_LOW) + pin / 16;

    if (output)
    {
        *dir = BIT(pin % 16) | BIT(pin % 16 + 16);
    }
    else
    {
        *dir = BIT(pin % 16 + 16);
    }
}

static inline void RK3588_WritePinOutput(volatile RK3588_GPIO_Type *base, uint32_t pin, uint8_t output)
{
    volatile uint32_t *data = ((volatile uint32_t *)(&base->DATA_LOW)) + pin / 16;

    if (output == 0U)
    {
        *data = BIT(pin % 16 + 16);
    }
    else
    {
        *data = BIT(pin % 16) | BIT(pin % 16 + 16);
    }
}

static inline void RK3588_SetPinsOutput(volatile RK3588_GPIO_Type *base, uint32_t pin)
{
    volatile uint32_t *data = ((volatile uint32_t *)(&base->DATA_LOW)) + pin / 16;

    *data = BIT(pin % 16) | BIT(pin % 16 + 16);
}

static inline void RK3588_ClearPinsOutput(volatile RK3588_GPIO_Type *base, uint32_t pin)
{
    volatile uint32_t *data = ((volatile uint32_t *)(&base->DATA_LOW)) + pin / 16;

    *data = BIT(pin % 16 + 16);
}

static inline uint32_t RK3588_ReadPinInput(volatile RK3588_GPIO_Type *base, uint32_t pin)
{
    return (((base->EXT_PORT) >> pin) & 0x01U);
}


#if defined(__cplusplus)
}
#endif