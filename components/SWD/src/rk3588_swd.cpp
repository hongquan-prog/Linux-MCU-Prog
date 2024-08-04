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
#include "rk3588_swd.h"

#define TAG "rk3588"

SWD_IO_OPERATIONS_DEFINE(RK3588_PinInit, RK3588_SetPinsOutput, RK3588_ClearPinsOutput, RK3588_ReadPinInput, RK3588_WritePinOutput)
SWD_TRANSFER_DEFINE(rk3399_swd_transfer)
SWJ_SEQUENCE_DEFINE(rk3399_swj_sequence)

RK3588SWD::RK3588SWD(RK3588_GPIO clk_port, int clk_pin, RK3588_GPIO dio_port, int dio_pin, uint32_t clock, bool remapping)
    : AH618SWD((AH618_GPIO)-1, -1, (AH618_GPIO)-1, -1, clock, false)
{
    if (remapping)
    {
        _clk.pin = clk_pin;
        _dio.pin = dio_pin;
        LOG_INFO("mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
        _clk.base = RK3588_GPIOBase(_dev_mem_fd, clk_port);
        LOG_INFO("swclk port base %p", _clk.base);
        _dio.base = RK3588_GPIOBase(_dev_mem_fd, dio_port);
        LOG_INFO("swdio port base %p", _dio.base);
        _delay = delay_calculate(2400 * 1000000U, clock, 1, 2, 1);
        LOG_INFO("swd_clock_delay: %d", _delay);
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
        PORT_INIT(&_clk, &_dio);
        _initialised = true;
    }
    return true;
}

bool RK3588SWD::off(void)
{
    if (_initialised)
    {
        PORT_OFF(&_clk, &_dio);
        _initialised = false;
    }

    return true;
}

SWDIface::transfer_err_def RK3588SWD::transer(uint32_t request, uint32_t *data)
{
    return static_cast<transfer_err_def>(rk3399_swd_transfer(&_clk, &_dio, _delay, request, data));
}

void RK3588SWD::swj_sequence(uint32_t count, const uint8_t *data)
{
    rk3399_swj_sequence(&_clk, &_dio, _delay, count, data);
}

void RK3588SWD::set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_nRESET_OUT(nullptr, 0) : PIN_nRESET_OUT(nullptr, 1);
}
