/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-8-4      lihongquan   adapted to Allwinner H618
 */
#include "ah6_swd.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"
#include "dap.h"
#include "ah6_gpio.h"

#define TAG "ah6"

AH6SWD::AH6SWD(int clk_pin, int dio_pin, uint32_t clock, bool remapping)
    : AH618SWD(-1,  -1, clock, false)
{
    if (remapping)
    {
        _clk.pin = clk_pin % 32;
        _dio.pin = dio_pin % 32;
        LOG_INFO("mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
        _clk.base = AH6_GPIOBase(_dev_mem_fd, clk_pin);
        LOG_INFO("swclk port base %p", _clk.base);
        _dio.base = AH6_GPIOBase(_dev_mem_fd, dio_pin);
        LOG_INFO("swdio port base %p", _dio.base);
        _delay = 0;
        LOG_INFO("clock delay: %d", _delay);
    }
}

AH6SWD::~AH6SWD()
{
    AH6_Clean();
    close(_dev_mem_fd);
}