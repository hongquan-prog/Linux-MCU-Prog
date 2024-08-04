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

#include "ah618_swd.h"
#include "rk3588_gpio.h"

class RK3588SWD : public AH618SWD
{
public:
    ~RK3588SWD();
    RK3588SWD(RK3588_GPIO clk_port, int clk_pin, RK3588_GPIO dio_port, int dio_pin, uint32_t clock, bool remapping = true);
    virtual bool init(void) override;
    virtual bool off(void) override;
    virtual transfer_err_def transer(uint32_t request, uint32_t *data) override;
    virtual void swj_sequence(uint32_t count, const uint8_t *data) override;
    virtual void set_target_reset(uint8_t asserted) override;
};
