/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-9-8      lihongquan   add license declaration
 */
#pragma once

#include "bin_program.h"
#include "hex_parser.h"
#include <cstdio>

class HexProgram : public BinaryProgram
{
private:
    uint8_t _decode_buffer[256];
    hex_parser_t _hex_parser;

    bool write_hex(const uint8_t *data, uint32_t size, uint32_t &decode_size);

public:
    HexProgram(SWDIface &swd);
    virtual bool init(const FlashIface::target_cfg_t &cfg, uint32_t program_addr) override;
    virtual bool write(uint8_t *data, size_t len) override;
};