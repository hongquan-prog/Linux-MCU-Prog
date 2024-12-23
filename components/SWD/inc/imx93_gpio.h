#pragma once

#include "compiler.h"
#include <stdbool.h>

typedef struct {
  __I  uint32_t VERID;                             /**< Version ID Register, offset: 0x0 */
  __I  uint32_t PARAM;                             /**< Parameter Register, offset: 0x4 */
       uint8_t RESERVED_0[4];
  __IO uint32_t LOCK;                              /**< Lock Register, offset: 0xC */
  __IO uint32_t PCNS;                              /**< Pin Control Non-Secure, offset: 0x10 */
  __IO uint32_t ICNS;                              /**< Interrupt Control Non-Secure, offset: 0x14 */
  __IO uint32_t PCNP;                              /**< Pin Control Non-Privilege, offset: 0x18 */
  __IO uint32_t ICNP;                              /**< Interrupt Control Non-Privilege, offset: 0x1C */
       uint8_t RESERVED_1[32];
  __IO uint32_t PDOR;                              /**< Port Data Output Register, offset: 0x40 */
  __O  uint32_t PSOR;                              /**< Port Set Output Register, offset: 0x44 */
  __O  uint32_t PCOR;                              /**< Port Clear Output Register, offset: 0x48 */
  __O  uint32_t PTOR;                              /**< Port Toggle Output Register, offset: 0x4C */
  __I  uint32_t PDIR;                              /**< Port Data Input Register, offset: 0x50 */
  __IO uint32_t PDDR;                              /**< Port Data Direction Register, offset: 0x54 */
  __IO uint32_t PIDR;                              /**< Port Input Disable Register, offset: 0x58 */
       uint8_t RESERVED_2[4];
  __IO uint8_t PDR[32];                            /**< Pin Data Register a, array offset: 0x60, array step: 0x1 */
  __IO uint32_t ICR[32];                           /**< Interrupt Control Register 0..Interrupt Control Register 31, array offset: 0x80, array step: 0x4 */
  __O  uint32_t GICLR;                             /**< Global Interrupt Control Low Register, offset: 0x100 */
  __O  uint32_t GICHR;                             /**< Global Interrupt Control High Register, offset: 0x104 */
       uint8_t RESERVED_3[24];
  __IO uint32_t ISFR[2];                           /**< Interrupt Status Flag Register, array offset: 0x120, array step: 0x4 */
} RGPIO_Type;


#if defined(__cplusplus)
extern "C" {
#endif

RGPIO_Type *RGPIO_GPIOBase(int fd, int pin);

void RGPIO_GPIOClean(void);

static inline void RGPIO_PinInit(volatile void *base, uint32_t pin, const bool output)
{
    if (output != true)
    {
        ((volatile RGPIO_Type *)base)->PDDR &= ~(1UL << pin);
    }
    else
    {
        ((volatile RGPIO_Type *)base)->PDDR |= (1UL << pin);
    }
}

static inline void RGPIO_WritePinOutput(volatile void *base, uint32_t pin, uint8_t output)
{
    if (output == 0U)
    {
        ((volatile RGPIO_Type *)base)->PCOR = 1UL << pin;
    }
    else
    {
        ((volatile RGPIO_Type *)base)->PSOR = 1UL << pin;
    }
}

static inline void RGPIO_SetPinsOutput(volatile void *base, uint32_t pin)
{
    ((volatile RGPIO_Type *)base)->PSOR = (1 << pin);
}

static inline void RGPIO_ClearPinsOutput(volatile void *base, uint32_t pin)
{
    ((volatile RGPIO_Type *)base)->PCOR = (1 << pin);
}

static inline uint32_t RGPIO_ReadPinInput(volatile void *base, uint32_t pin)
{
    return (((((volatile RGPIO_Type *)base)->PDIR) >> pin) & 0x01U);
}

#if defined(__cplusplus)
}
#endif