#include "sg2002_gpio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define GPIOA_BASE 0x03020000
#define GPIOB_BASE 0x03021000
#define GPIOC_BASE 0x03022000
#define GPIOD_BASE 0x03023000
#define GPIOPWR_BASE 0x05021000
#define GPIOAD_CFG_BASE 0x03001000
#define GPIOPWR_CFG_BASE 0x05027000

enum SG2002_GPIO
{
    SG2002_GPIOPWR = 11,
    SG2002_GPIOD = 12,
    SG2002_GPIOC = 13,
    SG2002_GPIOB = 14,
    SG2002_GPIOA = 15
};

static SG2002_GPIO_Type *gpio_mapped_addr[5] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL};

static uint32_t *gpioad_cfg_mapped_addr = NULL;
static uint32_t *gpiopwr_cfg_mapped_addr = NULL;

static const uint32_t gpio_base_addr[5] = {
    GPIOPWR_BASE,
    GPIOD_BASE,
    GPIOC_BASE,
    GPIOB_BASE,
    GPIOA_BASE};

static const uint32_t gpio_pad_cfg_addr[][2] = {
    // GPIOPWR
    {0x030010A4, 0x0502702C},
    {0, 0},
    {0, 0},
    {0x03001084, 0x05027008},
    {0x03001088, 0x0502700C},
    {0, 0},
    {0x03001090, 0x05027018},
    {0, 0},
    {0x03001098, 0x05027020},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0x030010D0, 0x05027058},
    {0x030010D4, 0x0502705C},
    {0x030010D8, 0x05027060},
    {0x030010DC, 0x05027064},
    {0x030010E0, 0x05027068},
    {0x030010E4, 0x0502706C},
    {0x030011D0, 0x050270E0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    // GPIOD
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    // GPIOC
    {0, 0},
    {0, 0},
    {0, 0},
    {0x0300116C, 0x03001C38},
    {0x03001170, 0x03001C3C},
    {0x03001174, 0x03001C40},
    {0x03001178, 0x03001C44},
    {0x0300117C, 0x03001C48},
    {0x03001180, 0x03001C4C},
    {0x03001184, 0x03001C50},
    {0x03001188, 0x03001C54},
    {0x0300118C, 0x03001C58},
    {0x03001190, 0x03001C5C},
    {0x030011B4, 0x03001C80},
    {0x030011B8, 0x03001C84},
    {0x030011AC, 0x03001C78},
    {0x030011B0, 0x03001C7C},
    {0x030011A4, 0x03001C70},
    {0x030011A8, 0x03001C74},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0x030011BC, 0},
    {0x030011C8, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    // GPIOB
    {0x030010EC, 0x03001804},
    {0, 0},
    {0, 0},
    {0x030010F8, 0x03001810},
    {0, 0},
    {0, 0},
    {0x03001108, 0x03001820},
    {0x03001118, 0},
    {0x03001114, 0},
    {0x03001120, 0},
    {0x0300111C, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0x030011CC, 0x03001C8C},
    {0x03001128, 0},
    {0x03001124, 0},
    {0x03001130, 0},
    {0x0300112C, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    // GPIOA
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {0x0300101C, 0x03001A00},
    {0x03001020, 0x03001A04},
    {0x03001024, 0x03001A08},
    {0x03001028, 0x03001A0C},
    {0x0300102C, 0x03001A10},
    {0x03001030, 0x03001A14},
    {0x03001034, 0x03001900},
    {0x03001038, 0x03001904},
    {0x0300103C, 0x03001908},
    {0x03001040, 0x0300190C},
    {0x03001044, 0x03001910},
    {0x03001068, 0x03001934},
    {0x03001064, 0x03001930},
    {0, 0},
    {0, 0},
    {0x03001050, 0x0300191C},
    {0x0300105C, 0x03001928},
    {0x03001060, 0x0300192C},
    {0x03001054, 0x03001920},
    {0x0300104C, 0x03001918},
    {0x03001058, 0x03001924},
    {0x03001070, 0x0300193C},
    {0x03001074, 0x03001940},
    {0x03001078, 0x03001944},
    {0, 0},
};

SG2002_GPIO_Type *SG2002_GPIOBase(int fd, int pin)
{
    int index = 0;
    int port = pin / 32;
    SG2002_GPIO_Type *ret = NULL;
    volatile uint32_t *pad_mapped_addr = NULL;
    volatile uint32_t *cfg_mapped_addr = NULL;

    if ((port == SG2002_GPIOA) || (port == SG2002_GPIOB) || (port == SG2002_GPIOC) || (port == SG2002_GPIOD) || (port == SG2002_GPIOPWR))
    {
        index = port - SG2002_GPIOPWR;
        if (!gpio_mapped_addr[index])
        {
            gpio_mapped_addr[index] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, gpio_base_addr[index]);
        }

        ret = gpio_mapped_addr[index];
        if ((SG2002_GPIOD <= port) && (SG2002_GPIOA >= port))
        {
            if (!gpioad_cfg_mapped_addr)
            {
                gpioad_cfg_mapped_addr = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOAD_CFG_BASE);
            }

            pad_mapped_addr = gpioad_cfg_mapped_addr;
            cfg_mapped_addr = gpioad_cfg_mapped_addr;
        }
        else if (SG2002_GPIOPWR == port)
        {
            if (!gpioad_cfg_mapped_addr)
            {
                gpioad_cfg_mapped_addr = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOAD_CFG_BASE);
            }

            if (!gpiopwr_cfg_mapped_addr)
            {
                gpiopwr_cfg_mapped_addr = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIOPWR_CFG_BASE);
            }

            pad_mapped_addr = gpioad_cfg_mapped_addr;
            cfg_mapped_addr = gpiopwr_cfg_mapped_addr;
        }
    }

    // enable pull up
    if (ret)
    {
        index = pin - SG2002_GPIOPWR * 32;
        volatile uint32_t *pad = pad_mapped_addr + ((gpio_pad_cfg_addr[index][0] - (gpio_pad_cfg_addr[index][0] & 0xFFFFF000)) / sizeof(uint32_t));
        volatile uint32_t *cfg = cfg_mapped_addr + ((gpio_pad_cfg_addr[index][1] - (gpio_pad_cfg_addr[index][1] & 0xFFFFF000)) / sizeof(uint32_t));

        if (pad)
        {
            *pad = 3;
        }

        if (cfg)
        {
            // output driving
            *cfg |= (7 << 5);
            // enable pull up
            *cfg |= (1 << 2);
            // disable pull down
            *cfg &= ~(1 << 3);
            // Output Buffer Slew Rate Limit disable
            *cfg &= ~(1 << 11);
        }
    }

    return ret;
}

void SG2002_Clean(void)
{
    for (int i = 0; i < sizeof(gpio_mapped_addr) / sizeof(gpio_mapped_addr[0]); i++)
    {
        if (gpio_mapped_addr[i])
        {
            munmap(gpio_mapped_addr[i], sysconf(_SC_PAGE_SIZE));
            gpio_mapped_addr[i] = NULL;
        }
    }

    if (gpiopwr_cfg_mapped_addr)
    {
        munmap(gpiopwr_cfg_mapped_addr, sysconf(_SC_PAGE_SIZE));
        gpiopwr_cfg_mapped_addr = NULL;
    }

    if (gpioad_cfg_mapped_addr)
    {
        munmap(gpioad_cfg_mapped_addr, sysconf(_SC_PAGE_SIZE));
        gpioad_cfg_mapped_addr = NULL;
    }
}