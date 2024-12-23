/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-9-8      lihongquan   add license declaration
 */
#include "log.h"
#include "bin_program.h"

#define TAG "bin_prog"

BinaryProgram::BinaryProgram(SWDIface &swd)
    : _flash(swd),
      _program_addr(0)
{
}

BinaryProgram::~BinaryProgram()
{
    _flash.uninit();
}

bool BinaryProgram::init(const FlashIface::target_cfg_t &cfg, uint32_t program_addr)
{
    if (!program_addr)
    {
        LOG_ERROR("The binary file must be provided with program address, %d", _program_addr);
        return false;
    }

    _program_addr = program_addr;
    LOG_INFO("Starting to program bin at 0x%x", _program_addr);

    return (_flash.init(cfg) == FlashIface::ERR_NONE);
}

bool BinaryProgram::write(uint8_t *data, size_t len)
{
    if (FlashIface::ERR_NONE != _flash.write(_program_addr, data, len))
    {
        LOG_ERROR("Failed to write data at:%x", _program_addr);
        return false;
    }

    _program_addr += len;

    return true;
}

size_t BinaryProgram::get_program_address(void)
{
    return _program_addr;
}

void BinaryProgram::clean()
{
    _program_addr = 0;
    _flash.uninit();
}