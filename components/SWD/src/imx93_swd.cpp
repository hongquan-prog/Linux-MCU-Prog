/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-9-8      lihongquan   add license declaration
 */
#include "imx93_swd.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"
#include "dap.h"
#include "sw_dp.h"
#include "compiler.h"

#define TAG "imx93"

static volatile int swclk_pin = -1;
static volatile int swdio_pin = -1;
static volatile uint32_t swd_clock_delay = 0;
static volatile RGPIO_Type *swclk_gpio = nullptr;
static volatile RGPIO_Type *swdio_gpio = nullptr;

#define GPIO_PIN_SWCLK swclk_pin
#define GPIO_PIN_SWDIO swdio_pin
#define SWCLK_GPIO_BASE swclk_gpio
#define SWDIO_GPIO_BASE swdio_gpio
SWD_PIN_OPERATIONS_DEFINE(RGPIO_PinInit, RGPIO_SetPinsOutput, RGPIO_ClearPinsOutput, RGPIO_ReadPinInput, RGPIO_WritePinOutput, SWCLK_GPIO_BASE, GPIO_PIN_SWCLK, SWDIO_GPIO_BASE, GPIO_PIN_SWDIO)

#undef PIN_DELAY
#define PIN_DELAY() PIN_DELAY_FAST()
SWD_TRANSFER_DEFINE(imx93_swd_transfer_fast)

#undef PIN_DELAY
#define PIN_DELAY() PIN_DELAY_SLOW(swd_clock_delay)
SWD_TRANSFER_DEFINE(imx93_swd_transfer_slow)

SWJ_SEQUENCE_DEFINE(imx93_swj_sequence)

IMX93SWD::IMX93SWD(FSL_GPIO clk_port, int clk_pin, FSL_GPIO dio_port, int dio_pin, bool remapping)
    : AH618SWD((AH618_GPIO)-1, -1, (AH618_GPIO)-1, -1, false)
{
    if (remapping)
    {
        swclk_pin = clk_pin;
        swdio_pin = dio_pin;
        LOG_INFO("mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
        SWCLK_GPIO_BASE = RGPIO_GPIOBase(_dev_mem_fd, clk_port);
        SWDIO_GPIO_BASE = RGPIO_GPIOBase(_dev_mem_fd, dio_port);
    }
}

IMX93SWD::~IMX93SWD()
{
    RGPIO_GPIOClean();
}

bool IMX93SWD::init(void)
{
    if (!_initialised)
    {
        PORT_INIT();
        swd_clock_delay = delay_calculate(1500 * 1000000U, DAP_DEFAULT_SWJ_CLOCK, 100, 4, 1);
        _fast_clock = (swd_clock_delay <= 1);
        _initialised = true;
    }
    return true;
}

bool IMX93SWD::off(void)
{
    if (_initialised)
    {
        PORT_OFF();
        _initialised = false;
    }

    return true;
}

SWDIface::transfer_err_def IMX93SWD::transer(uint32_t request, uint32_t *data)
{
    transfer_err_def ret;

    if (_fast_clock)
        ret = static_cast<transfer_err_def>(imx93_swd_transfer_fast(request, data));
    else
        ret = static_cast<transfer_err_def>(imx93_swd_transfer_slow(request, data));

    return ret;
}

void IMX93SWD::swj_sequence(uint32_t count, const uint8_t *data)
{
    imx93_swj_sequence(count, data);
}

void IMX93SWD::set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_nRESET_OUT(0) : PIN_nRESET_OUT(1);
}

IMX93SWD &IMX93SWD::instance(FSL_GPIO clk_port, int clk_pin, FSL_GPIO dio_port, int dio_pin)
{
    static IMX93SWD instance(clk_port, clk_pin, dio_port, dio_pin, true);
    return instance;
}