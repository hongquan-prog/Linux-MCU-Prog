#include "ah618_gpio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define GPIOA_BASE 0x0300B000
#define GPIOL_BASE 0x07022000

enum AH618_GPIO
{
    AH618_GPIOC = 2,
    AH618_GPIOF = 5,
    AH618_GPIOG = 6,
    AH618_GPIOH = 7,
    AH618_GPIOI = 8,
    AH618_GPIOL = 11
};

static AH618_GPIO_Type *gpio_mapped_addr[2] = {
    NULL,
    NULL};

AH618_GPIO_Type *AH618_GPIOBase(int fd, int pin)
{
    int port = pin / 32;
    AH618_GPIO_Type *ret = NULL;

    if ((port == AH618_GPIOC) || (port == AH618_GPIOF) || (port == AH618_GPIOG) || (port == AH618_GPIOH))
    {
        if (!gpio_mapped_addr[0])
        {
            gpio_mapped_addr[0] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOA_BASE);
            if (gpio_mapped_addr[0] == MAP_FAILED)
            {
                perror("mmap failed");
                exit(-1);
            }
        }

        ret = gpio_mapped_addr[0] + port;
    }
    else if (port == AH618_GPIOI)
    {
        if (!gpio_mapped_addr[0])
        {
            gpio_mapped_addr[0] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOA_BASE);
            if (gpio_mapped_addr[0] == MAP_FAILED)
            {
                perror("mmap failed");
                exit(-1);
            }
        }

        ret = gpio_mapped_addr[0] + 9;
    }
    else if (port == AH618_GPIOL)
    {
        if (!gpio_mapped_addr[1])
        {
            gpio_mapped_addr[1] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOL_BASE);
            if (gpio_mapped_addr[1] == MAP_FAILED)
            {
                perror("mmap failed");
                exit(-1);
            }
        }

        ret = gpio_mapped_addr[1];
    }

    // enable pull up
    if (ret)
    {
        pin = pin % 32;
        int offset = (pin % 16) * 2;
        volatile uint32_t *reg = ((volatile uint32_t *)(&ret->PUL0)) + pin / 16;

        *reg &= ~(0x3 << offset);
        *reg |= (1 << offset);
    }

    return ret;
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