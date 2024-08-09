#include "bcm2835_gpio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define GPIO_BASE 0x7E200000

static BCM2835_GPIO *gpio_mapped_addr[1] = {
    NULL};

BCM2835_GPIO *BCM2835_GPIOBase(int fd, int pin)
{
    if (!gpio_mapped_addr[0])
        gpio_mapped_addr[0] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);

    return gpio_mapped_addr[0];
}

void BCM2835_Clean(void)
{
    for (int i = 0; i < sizeof(gpio_mapped_addr) / sizeof(BCM2835_GPIO *); i++)
    {
        if (gpio_mapped_addr[i])
        {
            munmap(gpio_mapped_addr[i], sysconf(_SC_PAGE_SIZE));
            gpio_mapped_addr[i] = NULL;
        }
    }
}