#include "imx93_gpio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define GPIO1_BASE (0x47400000u)
#define GPIO2_BASE (0x43810000u)
#define GPIO3_BASE (0x43820000u)
#define GPIO4_BASE (0x43830000u)

static const uint32_t gpio_base[4] = {
    GPIO1_BASE,
    GPIO2_BASE,
    GPIO3_BASE,
    GPIO4_BASE};

static RGPIO_Type *gpio_mapped_addr[4] = {
    NULL,
    NULL,
    NULL,
    NULL};

RGPIO_Type *RGPIO_GPIOBase(int fd, enum FSL_GPIO gpio)
{
    if (gpio <= FSL_GPIO4)
    {
        if (!gpio_mapped_addr[gpio])
            gpio_mapped_addr[gpio] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, gpio_base[gpio]);

        return gpio_mapped_addr[gpio];
    }

    return NULL;
}