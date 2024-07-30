#include "log.h"
#include "imx_gpio.h"

#define TAG "imx_gpio"

static int s_init_gpio = 0;

void imx_init_swd_pins(void)
{
    if (!s_init_gpio)
    {
        s_init_gpio = 1;
        RGPIO_PinInit(GPIO_PORT_SWDIO, GPIO_PIN_SWDIO, kRGPIO_DigitalOutput);
        RGPIO_PinInit(GPIO_PORT_SWCLK, GPIO_PIN_SWCLK, kRGPIO_DigitalOutput);
        RGPIO_SetPinsOutput(GPIO_PORT_SWDIO, GPIO_PIN_SWDIO);
        RGPIO_SetPinsOutput(GPIO_PORT_SWCLK, GPIO_PIN_SWCLK);
        LOG_INFO("SWD GPIO init done");
    }
}

void imx_reset_dap_pins(void)
{
    if (s_init_gpio)
    {
        s_init_gpio = 0;
        RGPIO_PinInit(GPIO_PORT_SWDIO, GPIO_PIN_SWDIO, kRGPIO_DigitalInput);
        RGPIO_PinInit(GPIO_PORT_SWCLK, GPIO_PIN_SWCLK, kRGPIO_DigitalInput);
        LOG_INFO("SWD GPIO Reset done");
    }
}
