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
#include "ah618_swd.h"

class AH6SWD : public AH618SWD
{
public:
    ~AH6SWD();
    AH6SWD(int clk_pin, int dio_pin, uint32_t clock, bool remapping = true);
};
