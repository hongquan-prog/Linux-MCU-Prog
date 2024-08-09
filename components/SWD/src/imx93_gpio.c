#include "imx93_gpio.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

enum
{
	GPIO1,
	GPIO2,
	GPIO3,
	GPIO4,
	IOMUXC,
};

struct gpio_pad_info_t
{
	uint32_t pin;
	uint32_t value;
};

static const struct gpio_pad_info_t gpio_pads[] = {
	{124, 5},
	{125, 5},
	{126, 5},
	{127, 5},
	{64, 0},
	{65, 0},
	{66, 0},
	{67, 0},
	{68, 0},
	{69, 0},
	{70, 0},
	{71, 0},
	{72, 0},
	{73, 0},
	{74, 0},
	{75, 0},
	{76, 0},
	{77, 0},
	{78, 0},
	{79, 0},
	{80, 0},
	{81, 0},
	{82, 0},
	{83, 0},
	{84, 0},
	{85, 0},
	{86, 0},
	{87, 0},
	{88, 0},
	{89, 0},
	{90, 0},
	{91, 0},
	{92, 0},
	{93, 0},
	{122, 5},
	{123, 5},
	{156, 5},
	{157, 5},
	{128, 5},
	{129, 5},
	{130, 5},
	{131, 5},
	{132, 5},
	{133, 5},
	{134, 5},
	{135, 5},
	{136, 5},
	{137, 5},
	{138, 5},
	{139, 5},
	{140, 5},
	{141, 5},
	{142, 5},
	{143, 5},
	{144, 5},
	{145, 5},
	{146, 5},
	{147, 5},
	{148, 5},
	{149, 5},
	{150, 5},
	{151, 5},
	{152, 5},
	{153, 5},
	{154, 5},
	{155, 5},
	{104, 5},
	{105, 5},
	{106, 5},
	{107, 5},
	{108, 5},
	{109, 5},
	{110, 5},
	{111, 5},
	{112, 5},
	{113, 5},
	{114, 5},
	{115, 5},
	{116, 5},
	{117, 5},
	{118, 5},
	{119, 5},
	{120, 5},
	{121, 5},
	{96, 5},
	{97, 5},
	{98, 5},
	{99, 5},
	{100, 5},
	{101, 5},
	{102, 5},
	{103, 5},
	{32, 5},
	{33, 5},
	{34, 5},
	{35, 5},
	{36, 5},
	{37, 5},
	{38, 5},
	{39, 5},
	{40, 5},
	{41, 5},
	{42, 5},
	{43, 5},
	{44, 5},
	{45, 5},
	{46, 5},
	{47, 5}};

static const uint32_t reg_base[5] = {
	0x47400000u,
	0x43810000u,
	0x43820000u,
	0x43830000u,
	0x443C0000u};

static uint32_t *mapped_addr[5] = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL};

RGPIO_Type *RGPIO_GPIOBase(int fd, int pin)
{
	int port = pin / 32;
	volatile uint32_t *pad = NULL;
	RGPIO_Type *ret = NULL;

	if (!mapped_addr[IOMUXC])
	{
		mapped_addr[IOMUXC] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, reg_base[IOMUXC]);
	}

	// MUX
	for (unsigned int i = 0; i < sizeof(gpio_pads) / sizeof(struct gpio_pad_info_t); i++)
	{
		if ((gpio_pads[i].pin == pin) && mapped_addr[4])
		{
			// MUX Mode Select GPIO
			pad = ((volatile uint32_t *)mapped_addr[4]) + i;
			*pad &= ~(0x7);
			*pad |= gpio_pads[i].value;

			// enable pull up
			pad = ((volatile uint32_t *)mapped_addr[4]) + i + (sizeof(gpio_pads) / sizeof(struct gpio_pad_info_t));
			*pad &= ~(3 << 9);
			*pad |= 1 << 9;
		}
	}

	if (port < IOMUXC)
	{
		if (!mapped_addr[port])
			mapped_addr[port] = mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, fd, reg_base[port]);

		ret = (RGPIO_Type *)mapped_addr[port];
	}

	return ret;
}

void RGPIO_GPIOClean(void)
{
	for (int i = 0; i <= IOMUXC; i++)
	{
		if (mapped_addr[i])
		{
			munmap(mapped_addr[i], sysconf(_SC_PAGE_SIZE));
			mapped_addr[i] = NULL;
		}
	}
}