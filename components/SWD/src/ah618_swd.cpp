/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-8-4      lihongquan   adapted to Allwinner H618
 */
#include "ah618_swd.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"
#include "dap.h"
#include "ah618_gpio.h"

#define TAG "ah618"

SWD_IO_OPERATIONS_DEFINE(AH618_PinInit, AH618_SetPinsOutput, AH618_ClearPinsOutput, AH618_ReadPinInput, AH618_WritePinOutput)
SWD_TRANSFER_DEFINE(awh6_swd_transfer)
SWJ_SEQUENCE_DEFINE(awh6_swj_sequence)

AH618SWD::AH618SWD(int clk_pin, int dio_pin, uint32_t clock, bool remapping, const std::string &mem)
    : _initialised(false),
      _delay(0),
      _clk{nullptr, 0},
      _dio{nullptr, 0}
{
    _dev_mem_fd = open(mem.c_str(), O_RDWR | O_SYNC);
    if (_dev_mem_fd < 0)
    {
        LOG_ERROR("open: %s", strerror(errno));
        exit(-1);
    }

    if (remapping)
    {
        _clk.pin = clk_pin % 32;
        _dio.pin = dio_pin % 32;
        LOG_INFO("mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
        _clk.base = AH618_GPIOBase(_dev_mem_fd, clk_pin);
        LOG_INFO("swclk port base %p", _clk.base);
        _dio.base = AH618_GPIOBase(_dev_mem_fd, dio_pin);
        LOG_INFO("swdio port base %p", _dio.base);
        _delay = 0;
        LOG_INFO("clock delay: %d", _delay);
    }
}

AH618SWD::~AH618SWD()
{
    AH618_Clean();
    close(_dev_mem_fd);
}

void AH618SWD::msleep(uint32_t ms)
{
    usleep(1000 * ms);
}

bool AH618SWD::init(void)
{
    if (!_initialised)
    {
        PORT_INIT(&_clk, &_dio);
        _initialised = true;
    }

    return true;
}

bool AH618SWD::off(void)
{
    if (_initialised)
    {
        PORT_OFF(&_clk, &_dio);
        _initialised = false;
    }
    return true;
}

SWDIface::transfer_err_def AH618SWD::transer(uint32_t request, uint32_t *data)
{
    return static_cast<transfer_err_def>(awh6_swd_transfer(&_clk, &_dio, _delay, request, data));
}

void AH618SWD::swj_sequence(uint32_t count, const uint8_t *data)
{
    awh6_swj_sequence(&_clk, &_dio, _delay, count, data);
}

void AH618SWD::set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_nRESET_OUT(nullptr, 0) : PIN_nRESET_OUT(nullptr, 1);
}