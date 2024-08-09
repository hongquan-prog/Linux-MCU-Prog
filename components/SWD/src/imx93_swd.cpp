/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-8-4      lihongquan   adapted to imx9352
 */

#include "imx93_swd.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"
#include "dap.h"
#include "imx93_gpio.h"

#define TAG "imx93"

SWD_IO_OPERATIONS_DEFINE(RGPIO_PinInit, RGPIO_SetPinsOutput, RGPIO_ClearPinsOutput, RGPIO_ReadPinInput, RGPIO_WritePinOutput)
SWD_TRANSFER_DEFINE(imx93_swd_transfer)
SWJ_SEQUENCE_DEFINE(imx93_swj_sequence)

IMX93SWD::IMX93SWD(int clk_pin, int dio_pin, uint32_t clock, bool remapping)
    : AH618SWD(-1, -1, clock, false)
{
    if (remapping)
    {
        _clk.pin = clk_pin % 32;
        _dio.pin = dio_pin % 32;
        LOG_INFO("mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
        _clk.base = RGPIO_GPIOBase(_dev_mem_fd, clk_pin);
        LOG_INFO("swclk port base %p", _clk.base);
        _dio.base = RGPIO_GPIOBase(_dev_mem_fd, dio_pin);
        LOG_INFO("swdio port base %p", _dio.base);
        _delay = delay_calculate(1500 * 1000000U, clock, 100, 4, 1);
        LOG_INFO("clock delay: %d", _delay);
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
        PORT_INIT(&_clk, &_dio);
        _initialised = true;
    }
    return true;
}

bool IMX93SWD::off(void)
{
    if (_initialised)
    {
        PORT_OFF(&_clk, &_dio);
        _initialised = false;
    }

    return true;
}

SWDIface::transfer_err_def IMX93SWD::transer(uint32_t request, uint32_t *data)
{
    return static_cast<transfer_err_def>(imx93_swd_transfer(&_clk, &_dio, _delay, request, data));
}

void IMX93SWD::swj_sequence(uint32_t count, const uint8_t *data)
{
    imx93_swj_sequence(&_clk, &_dio, _delay, count, data);
}

void IMX93SWD::set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_nRESET_OUT(nullptr, 0) : PIN_nRESET_OUT(nullptr, 1);
}