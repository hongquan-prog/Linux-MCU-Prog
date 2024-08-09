#include "ah6_gpio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define GPIOA_BASE 0x0300B000
#define GPIOL_BASE 0x07022000

enum AH6_GPIO
{
    AH6_GPIOC = 2,
    AH6_GPIOD = 3,
    AH6_GPIOF = 5,
    AH6_GPIOG = 6,
    AH6_GPIOH = 7,
    AH6_GPIOL = 11,
    AH6_GPIOM = 12
};

static AH618_GPIO_Type *gpio_mapped_addr[2] = {
    NULL,
    NULL};

AH618_GPIO_Type *AH6_GPIOBase(int fd, int pin)
{
    int port = pin / 32;
    AH618_GPIO_Type *ret = NULL;

    if ((port == AH6_GPIOC) || (port == AH6_GPIOD) || (port == AH6_GPIOF) || (port == AH6_GPIOG) || (port == AH6_GPIOH))
    {
        if (!gpio_mapped_addr[0])
            gpio_mapped_addr[0] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOA_BASE);
        ret = gpio_mapped_addr[0] + port;
    }
    else if ((port == AH6_GPIOL) || (port == AH6_GPIOM))
    {
        if (!gpio_mapped_addr[1])
            gpio_mapped_addr[1] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOL_BASE);
        ret = gpio_mapped_addr[1] + (port - AH6_GPIOL);
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

void AH6_Clean(void)
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