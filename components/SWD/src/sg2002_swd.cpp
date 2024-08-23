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
#include "sg2002_swd.h"
#include "sg2002_gpio.h"

#define TAG "SG2002"

SWD_IO_OPERATIONS_DEFINE(SG2002_PinInit, SG2002_SetPinsOutput, SG2002_ClearPinsOutput, SG2002_ReadPinInput, SG2002_WritePinOutput)
SWD_TRANSFER_DEFINE(sg2002_swd_transfer)
SWJ_SEQUENCE_DEFINE(sg2002_swj_sequence)

SG2002SWD::SG2002SWD(int clk_pin, int dio_pin, uint32_t clock, bool remapping)
    : AH618SWD(-1, -1, clock, false)
{
    if (remapping)
    {
        _clk.pin = clk_pin % 32;
        _dio.pin = dio_pin % 32;
        LOG_INFO("mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
        _clk.base = SG2002_GPIOBase(_dev_mem_fd, clk_pin);
        LOG_INFO("swclk port base %p", _clk.base);
        _dio.base = SG2002_GPIOBase(_dev_mem_fd, dio_pin);
        LOG_INFO("swdio port base %p", _dio.base);
        _delay = 0;
        LOG_INFO("swd_clock_delay: %d", _delay);
    }
}

SG2002SWD::~SG2002SWD()
{
    SG2002_Clean();
}

bool SG2002SWD::init(void)
{
    if (!_initialised)
    {
        PORT_INIT(&_clk, &_dio);
        _initialised = true;
    }
    return false;
}

bool SG2002SWD::off(void)
{
    if (_initialised)
    {
        PORT_OFF(&_clk, &_dio);
        _initialised = false;
    }

    return true;
}

SWDIface::transfer_err_def SG2002SWD::transer(uint32_t request, uint32_t *data)
{
    return static_cast<transfer_err_def>(sg2002_swd_transfer(&_clk, &_dio, _delay, request, data));
}

void SG2002SWD::swj_sequence(uint32_t count, const uint8_t *data)
{
    sg2002_swj_sequence(&_clk, &_dio, _delay, count, data);
}

void SG2002SWD::set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_nRESET_OUT(nullptr, 0) : PIN_nRESET_OUT(nullptr, 1);
}
