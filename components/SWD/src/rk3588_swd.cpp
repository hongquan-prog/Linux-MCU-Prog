/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-9-8      lihongquan   add license declaration
 */
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"
#include "dap.h"
#include "sw_dp.h"
#include "compiler.h"
#include "rk3588_swd.h"

#define TAG "rk3588"

static volatile int swclk_pin = -1;
static volatile int swdio_pin = -1;
static volatile uint32_t swd_clock_delay = 500;
static volatile RK3588_GPIO_Type *swclk_gpio = nullptr;
static volatile RK3588_GPIO_Type *swdio_gpio = nullptr;

#define GPIO_PIN_SWCLK swclk_pin
#define GPIO_PIN_SWDIO swdio_pin
#define SWCLK_GPIO_BASE swclk_gpio
#define SWDIO_GPIO_BASE swdio_gpio
SWD_PIN_OPERATIONS_DEFINE(RK3588_PinInit, RK3588_SetPinsOutput, RK3588_ClearPinsOutput, RK3588_ReadPinInput, RK3588_WritePinOutput, SWCLK_GPIO_BASE, GPIO_PIN_SWCLK, SWDIO_GPIO_BASE, GPIO_PIN_SWDIO)

#undef PIN_DELAY
#define PIN_DELAY() PIN_DELAY_FAST()
SWD_TRANSFER_DEFINE(rk3399_swd_transfer_fast)

#undef PIN_DELAY
#define PIN_DELAY() PIN_DELAY_SLOW(swd_clock_delay)
SWD_TRANSFER_DEFINE(rk3399_swd_transfer_slow)

SWJ_SEQUENCE_DEFINE(rk3399_swj_sequence)

RK3588SWD::RK3588SWD(RK3588_GPIO clk_port, int clk_pin, RK3588_GPIO dio_port, int dio_pin, bool remapping)
    : AH618SWD((AH618_GPIO)-1, -1, (AH618_GPIO)-1, -1, false)
{
    if (remapping)
    {
        swclk_pin = clk_pin;
        swdio_pin = dio_pin;
        LOG_INFO("mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
        SWCLK_GPIO_BASE = RK3588_GPIOBase(_dev_mem_fd, clk_port);
        LOG_INFO("SWCLK_GPIO_BASE %p", SWCLK_GPIO_BASE);
        SWDIO_GPIO_BASE = RK3588_GPIOBase(_dev_mem_fd, dio_port);
        LOG_INFO("SWDIO_GPIO_BASE %p", SWDIO_GPIO_BASE);
    }
}

RK3588SWD::~RK3588SWD()
{
    RK3588_Clean();
}

bool RK3588SWD::init(void)
{
    if (!_initialised)
    {
        PORT_INIT();
        swd_clock_delay = delay_calculate(2400 * 1000000U, 4000000, 1, 2, 1);
        LOG_INFO("swd_clock_delay: %d", swd_clock_delay);
        _fast_clock = (swd_clock_delay <= 1);
        _initialised = true;
    }
    return true;
}

bool RK3588SWD::off(void)
{
    if (_initialised)
    {
        PORT_OFF();
        _initialised = false;
    }

    return true;
}

SWDIface::transfer_err_def RK3588SWD::transer(uint32_t request, uint32_t *data)
{
    transfer_err_def ret;

    if (_fast_clock)
        ret = static_cast<transfer_err_def>(rk3399_swd_transfer_fast(request, data));
    else
        ret = static_cast<transfer_err_def>(rk3399_swd_transfer_slow(request, data));

    return ret;
}

void RK3588SWD::swj_sequence(uint32_t count, const uint8_t *data)
{
    rk3399_swj_sequence(count, data);
}

void RK3588SWD::set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_nRESET_OUT(0) : PIN_nRESET_OUT(1);
}

RK3588SWD &RK3588SWD::instance(RK3588_GPIO clk_port, int clk_pin, RK3588_GPIO dio_port, int dio_pin)
{
    static RK3588SWD instance(clk_port, clk_pin, dio_port, dio_pin, true);
    return instance;
}