/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-9-8      lihongquan   add license declaration
 */
#include "target_swd.h"
#include "DAP.h"
#include <unistd.h>

extern "C" uint8_t swd_init(void);
extern "C" uint8_t swd_off(void);
extern "C" void swd_set_target_reset(uint8_t asserted);

void TargetSWD::msleep(uint32_t ms)
{
    usleep(1000 * ms);
}

bool TargetSWD::init(void)
{
    return swd_init();
}

bool TargetSWD::off(void)
{
    return swd_off();
}

SWDIface::transfer_err_def TargetSWD::transer(uint32_t request, uint32_t *data)
{
    return static_cast<SWDIface::transfer_err_def>(SWD_Transfer(request, data));
}

void TargetSWD::swj_sequence(uint32_t count, const uint8_t *data)
{
    SWJ_Sequence(count, data);
}

void TargetSWD::set_target_reset(uint8_t asserted)
{
    swd_set_target_reset(asserted);
}

TargetSWD &TargetSWD::get_instance()
{
    static TargetSWD instance;
    return instance;
}