/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-7-31      lihongquan   add license declaration
 */
#pragma once

#include "sw_dp.h"
#include "swd_iface.h"
#include "ah618_gpio.h"

class AH618SWD : public SWDIface
{
protected:
    bool _initialised;
    int _dev_mem_fd;
    uint32_t _delay;
    volatile swd_pin_t _clk;
    volatile swd_pin_t _dio;

public:
    ~AH618SWD();
    AH618SWD(AH618_GPIO clk_port, int clk_pin, AH618_GPIO dio_port, int dio_pin, uint32_t clock, bool remapping = true);
    virtual void msleep(uint32_t ms) override;
    virtual bool init(void) override;
    virtual bool off(void) override;
    virtual transfer_err_def transer(uint32_t request, uint32_t *data) override;
    virtual void swj_sequence(uint32_t count, const uint8_t *data) override;
    virtual void set_target_reset(uint8_t asserted) override;
};
