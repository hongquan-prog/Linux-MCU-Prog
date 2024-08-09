/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-8-4      lihongquan   adapted to rk3358
 */
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"
#include "dap.h"
#include "compiler.h"
#include "bcm2835_swd.h"
#include "bcm2835_gpio.h"

#define TAG "BCM2835"

SWD_IO_OPERATIONS_DEFINE(BCM2835_PinInit, BCM2835_SetPinsOutput, BCM2835_ClearPinsOutput, BCM2835_ReadPinInput, BCM2835_WritePinOutput)
SWD_TRANSFER_DEFINE(bcm2835_swd_transfer)
SWJ_SEQUENCE_DEFINE(bcm2835_swj_sequence)

BCM2835SWD::BCM2835SWD(int clk_pin, int dio_pin, uint32_t clock, bool remapping)
    : AH618SWD(-1, -1, clock, false, "/dev/gpiomem")
{
    if (remapping)
    {
        _clk.pin = clk_pin;
        _dio.pin = dio_pin;
        LOG_INFO("mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
        _clk.base = BCM2835_GPIOBase(_dev_mem_fd, clk_pin);
        LOG_INFO("swclk port base %p", _clk.base);
        _dio.base = BCM2835_GPIOBase(_dev_mem_fd, dio_pin);
        LOG_INFO("swdio port base %p", _dio.base);
        _delay = delay_calculate(1200 * 1000000U, clock, 10, 3, 1);
        LOG_INFO("swd_clock_delay: %d", _delay);
    }
}

BCM2835SWD::~BCM2835SWD()
{
    BCM2835_Clean();
}

bool BCM2835SWD::init(void)
{
    if (!_initialised)
    {
        PORT_INIT(&_clk, &_dio);
        _initialised = true;
    }
    return true;
}

bool BCM2835SWD::off(void)
{
    if (_initialised)
    {
        PORT_OFF(&_clk, &_dio);
        _initialised = false;
    }

    return true;
}

SWDIface::transfer_err_def BCM2835SWD::transer(uint32_t request, uint32_t *data)
{
    return static_cast<transfer_err_def>(bcm2835_swd_transfer(&_clk, &_dio, _delay, request, data));
}

void BCM2835SWD::swj_sequence(uint32_t count, const uint8_t *data)
{
    bcm2835_swj_sequence(&_clk, &_dio, _delay, count, data);
}

void BCM2835SWD::set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_nRESET_OUT(nullptr, 0) : PIN_nRESET_OUT(nullptr, 1);
}
