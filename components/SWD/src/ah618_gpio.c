#include "ah618_gpio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define GPIOA_BASE 0x0300B000
#define GPIOL_BASE 0x07022000

static AH618_GPIO_Type *gpio_mapped_addr[2] = {
    NULL,
    NULL};

AH618_GPIO_Type *AH618_GPIOBase(int fd, enum AH618_GPIO gpio)
{
    if (gpio <= AH618_GPIOI)
    {
        if (gpio <= AH618_GPIOH)
        {
            if (!gpio_mapped_addr[0])
                gpio_mapped_addr[0] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOA_BASE);
            return gpio_mapped_addr[0] + gpio;
        }
        else
        {
            if (!gpio_mapped_addr[1])
                gpio_mapped_addr[1] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOL_BASE);
            return gpio_mapped_addr[1] + gpio;
        }
    }

    return NULL;
}

void AH618_Clean(void)
{
    for (int i = 0; i < 2; i++)
    {
        if (gpio_mapped_addr[i])
        {
            munmap(gpio_mapped_addr[i], sysconf(_SC_PAGE_SIZE));
            gpio_mapped_addr[i] = NULL;
        }
    }
}