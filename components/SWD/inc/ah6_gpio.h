#pragma once

#include "ah618_gpio.h"

#if defined(__cplusplus)
extern "C" {
#endif

AH618_GPIO_Type *AH6_GPIOBase(int fd, int pin);
void AH6_Clean(void);

#if defined(__cplusplus)
}
#endif