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
#include "imx93_gpio.h"

class IMX93SWD : public AH618SWD
{
protected:
    IMX93SWD(FSL_GPIO clk_port, int clk_pin, FSL_GPIO dio_port, int dio_pin, bool remapping = true);

public:
    ~IMX93SWD();
    static IMX93SWD &instance(FSL_GPIO clk_port, int clk_pin, FSL_GPIO dio_port, int dio_pin);
    virtual bool init(void) override;
    virtual bool off(void) override;
    virtual transfer_err_def transer(uint32_t request, uint32_t *data) override;
    virtual void swj_sequence(uint32_t count, const uint8_t *data) override;
    virtual void set_target_reset(uint8_t asserted) override;
};
