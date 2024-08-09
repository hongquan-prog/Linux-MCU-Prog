/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-8-9       lihongquan   adapted to raspberry pi3
 */
#pragma once

#include "ah618_swd.h"

class BCM2835SWD : public AH618SWD
{
public:
    ~BCM2835SWD();
    BCM2835SWD(int clk_pin, int dio_pin, uint32_t clock, bool remapping = true);
    virtual bool init(void) override;
    virtual bool off(void) override;
    virtual transfer_err_def transer(uint32_t request, uint32_t *data) override;
    virtual void swj_sequence(uint32_t count, const uint8_t *data) override;
    virtual void set_target_reset(uint8_t asserted) override;
};
