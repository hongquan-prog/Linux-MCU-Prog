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

#include "target_flash.h"

class FlashAccessor : public TargetFlash
{
private:
    FlashIface::state_t _flash_state;
    bool _current_sector_valid;
    uint32_t _last_packet_addr = 0;
    uint32_t _current_write_block_addr;
    uint32_t _current_write_block_size;
    uint32_t _current_sector_addr;
    uint32_t _current_sector_size;
    bool _page_buf_empty;
    std::shared_ptr<uint8_t []> _page_buffer;

    FlashIface::err_t compare_flush_current_block(void);
    FlashIface::err_t flush_current_block(uint32_t addr);
    FlashIface::err_t setup_next_sector(uint32_t addr);

public:
    FlashAccessor(SWDIface &swd);
    ~FlashAccessor() = default;
    FlashIface::err_t init(const target_cfg_t &cfg);
    FlashIface::err_t write(uint32_t addr, const uint8_t *data, uint32_t size);
    FlashIface::err_t uninit();
};
