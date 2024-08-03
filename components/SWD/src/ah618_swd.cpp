/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-9-8      lihongquan   add license declaration
 */
#include "ah618_swd.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include "log.h"
#include "dap.h"
#include "sw_dp.h"

#define TAG "ah618"

static volatile int swclk_pin = -1;
static volatile int swdio_pin = -1;
static volatile uint32_t swd_clock_delay = 0;
static volatile AH618_GPIO_Type *swclk_gpio = nullptr;
static volatile AH618_GPIO_Type *swdio_gpio = nullptr;

#define GPIO_PIN_SWCLK swclk_pin
#define GPIO_PIN_SWDIO swdio_pin
#define SWCLK_GPIO_BASE swclk_gpio
#define SWDIO_GPIO_BASE swdio_gpio
SWD_PIN_OPERATIONS_DEFINE(AH618_PinInit, AH618_SetPinsOutput, AH618_ClearPinsOutput, AH618_ReadPinInput, AH618_WritePinOutput, SWCLK_GPIO_BASE, GPIO_PIN_SWCLK, SWDIO_GPIO_BASE, GPIO_PIN_SWDIO)

#undef PIN_DELAY
#define PIN_DELAY() PIN_DELAY_FAST()
SWD_TRANSFER_DEFINE(awh6_swd_transfer_fast)

#undef PIN_DELAY
#define PIN_DELAY() PIN_DELAY_SLOW(swd_clock_delay)
SWD_TRANSFER_DEFINE(awh6_swd_transfer_slow)

SWJ_SEQUENCE_DEFINE(awh6_swj_sequence)

AH618SWD::AH618SWD(AH618_GPIO clk_port, int clk_pin, AH618_GPIO dio_port, int dio_pin, bool remapping)
    : _fast_clock(false),
      _initialised(false)
{
    _dev_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (_dev_mem_fd < 0)
    {
        LOG_ERROR("open: %s", strerror(errno));
        exit(-1);
    }

    if (remapping)
    {
        swclk_pin = clk_pin;
        swdio_pin = dio_pin;
        LOG_INFO("mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
        SWCLK_GPIO_BASE = AH618_GPIOBase(_dev_mem_fd, clk_port);
        LOG_INFO("SWCLK_GPIO_BASE %p", SWCLK_GPIO_BASE);
        SWDIO_GPIO_BASE = AH618_GPIOBase(_dev_mem_fd, dio_port);
        LOG_INFO("SWDIO_GPIO_BASE %p", SWDIO_GPIO_BASE);
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
        PORT_INIT();
        swd_clock_delay = 0;
        _fast_clock = true;
        _initialised = true;
    }

    return true;
}

bool AH618SWD::off(void)
{
    if (_initialised)
    {
        PORT_OFF();
        _initialised = false;
    }
    return true;
}

SWDIface::transfer_err_def AH618SWD::transer(uint32_t request, uint32_t *data)
{
    transfer_err_def ret;

    if (_fast_clock)
        ret = static_cast<transfer_err_def>(awh6_swd_transfer_fast(request, data));
    else
        ret = static_cast<transfer_err_def>(awh6_swd_transfer_slow(request, data));

    return ret;
}

void AH618SWD::swj_sequence(uint32_t count, const uint8_t *data)
{
    awh6_swj_sequence(count, data);
}

void AH618SWD::set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_nRESET_OUT(0) : PIN_nRESET_OUT(1);
}

AH618SWD &AH618SWD::instance(AH618_GPIO clk_port, int clk_pin, AH618_GPIO dio_port, int dio_pin)
{
    static AH618SWD instance(clk_port, clk_pin, dio_port, dio_pin, true);
    return instance;
}