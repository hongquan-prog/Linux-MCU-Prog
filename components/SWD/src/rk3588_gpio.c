#include "rk3588_gpio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

enum
{
    RK3588_GPIO0,
    RK3588_GPIO1,
    RK3588_GPIO2,
    RK3588_GPIO3,
    RK3588_GPIO4
};

static const uint32_t gpio_base[] = {
    0xFD8A0000,
    0xFEC20000,
    0xFEC30000,
    0xFEC40000,
    0xFEC50000};

static RK3588_GPIO_Type *gpio_mapped_addr[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL};

RK3588_GPIO_Type *RK3588_GPIOBase(int fd, int pin)
{
    int port = pin / 32;

    if (port <= RK3588_GPIO4)
    {
        if (!gpio_mapped_addr[port])
            gpio_mapped_addr[port] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, gpio_base[port]);
        return gpio_mapped_addr[port];
    }

    return NULL;
}

void RK3588_Clean(void)
{
    for (int i = 0; i < 5; i++)
    {
        if (gpio_mapped_addr[i])
        {
            munmap(gpio_mapped_addr[i], sysconf(_SC_PAGE_SIZE));
            gpio_mapped_addr[i] = NULL;
        }
    }
}