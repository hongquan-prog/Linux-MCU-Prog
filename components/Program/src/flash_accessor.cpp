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
#include "flash_accessor.h"
#include <cstring>

#define TAG "flash"
#define ROUND_UP(value, boundary) ((value) + ((boundary) - (value)) % (boundary))
#define ROUND_DOWN(value, boundary) ((value) - ((value) % (boundary)))
#define CONFIG_STATIC_PAGE_BUFFER_SIZE 0

FlashAccessor::FlashAccessor(SWDIface &swd)
    : TargetFlash(swd),
      _flash_state(FLASH_STATE_CLOSED),
      _current_sector_valid(false),
      _last_packet_addr(0),
      _current_write_block_addr(0),
      _current_write_block_size(0),
      _current_sector_addr(0),
      _current_sector_size(0),
      _page_buf_empty(true),
      _page_buffer(nullptr)
{
#if defined(CONFIG_STATIC_PAGE_BUFFER_SIZE) && (CONFIG_STATIC_PAGE_BUFFER_SIZE > 0)
    _page_buffer = std::shared_ptr<uint8_t[]>(new uint8_t[CONFIG_STATIC_PAGE_BUFFER_SIZE]);
#endif
}

FlashIface::err_t FlashAccessor::compare_flush_current_block(void)
{
    FlashIface::err_t status = ERR_NONE;
    uint32_t verify_bolck_size = 0;
    uint8_t *verify_data = nullptr;
    uint32_t verify_addr = 0;
    bool bolck_is_same = true;
    bool flash_is_empty = true;

    // check the content of flash
    verify_data = _page_buffer.get();
    verify_addr = _current_write_block_addr;
    verify_bolck_size = _current_write_block_size;
    while (verify_bolck_size > 0)
    {
        uint32_t verify_size = (verify_bolck_size <= sizeof(_verify_buf)) ? (verify_bolck_size) : (sizeof(_verify_buf));

        if (!_swd.read_memory(verify_addr, _verify_buf, verify_size))
        {
            LOG_ERROR("Error reading flash buffer");
            return ERR_ALGO_DATA_SEQ;
        }

        if (bolck_is_same && (memcmp(verify_data, _verify_buf, verify_size) != 0))
        {
            bolck_is_same = false;
        }

        if (flash_is_empty)
        {
            for (uint32_t i = 0; i < verify_size; i++)
            {
                if (_verify_buf[i] != 0xff)
                {
                    flash_is_empty = false;
                }
            }
        }

        if (!bolck_is_same && !flash_is_empty)
        {
            break;
        }

        verify_addr += verify_size;
        verify_data += verify_size;
        verify_bolck_size -= verify_size;
    }

    if (!bolck_is_same)
    {
        if (!flash_is_empty)
        {
            // Erase the current sector
            status = flash_erase_sector(_current_sector_addr);
            if (ERR_NONE != status)
            {
                LOG_ERROR("Flash sector erase failed");
                flash_uninit();
                return status;
            }
        }

        status = flash_program_page(_current_write_block_addr, _page_buffer.get(), _current_write_block_size);
    }

    return status;
}

FlashIface::err_t FlashAccessor::flush_current_block(uint32_t addr)
{
    FlashIface::err_t status = ERR_NONE;

    // Write out current buffer if there is data in it
    if (!_page_buf_empty)
    {
#if defined(CONFIG_STATIC_PAGE_BUFFER_SIZE) && (CONFIG_STATIC_PAGE_BUFFER_SIZE > 0)
        if (_current_write_block_addr == _current_sector_addr)
        {
            // Erase the current sector
            status = flash_erase_sector(_current_sector_addr);
            if (ERR_NONE != status)
            {
                LOG_ERROR("Flash sector erase failed");
                flash_uninit();
                return status;
            }
        }

        status = flash_program_page(_current_write_block_addr, _page_buffer.get(), _current_write_block_size);
#else
        status = compare_flush_current_block();
#endif
        compare_flush_current_block();
        _page_buf_empty = true;
    }

    // Setup for next block
    memset(_page_buffer.get(), 0xFF, _current_write_block_size);
    if (!_current_write_block_size)
    {
        return ERR_INTERNAL;
    }

    _current_write_block_addr = ROUND_DOWN(addr, _current_write_block_size);

    return status;
}

FlashIface::err_t FlashAccessor::setup_next_sector(uint32_t addr)
{
    uint32_t min_prog_size = 0;
    uint32_t sector_size = 0;
    FlashIface::err_t status = ERR_NONE;

    min_prog_size = flash_program_page_min_size(addr);
    sector_size = flash_erase_sector_size(addr);
    if ((min_prog_size <= 0) || (sector_size <= 0))
    {
        return ERR_INTERNAL;
    }

    // Setup global variables
    _current_sector_addr = ROUND_DOWN(addr, sector_size);
    _current_sector_size = sector_size;
    _current_write_block_addr = _current_sector_addr;
#if defined(CONFIG_STATIC_PAGE_BUFFER_SIZE) && (CONFIG_STATIC_PAGE_BUFFER_SIZE > 0)
    _current_write_block_size = (sector_size <= CONFIG_STATIC_PAGE_BUFFER_SIZE) ? (sector_size) : (CONFIG_STATIC_PAGE_BUFFER_SIZE);
#else
    // realloc buffer if the bock size changed
    if (_current_write_block_size != _current_sector_size)
    {
        _page_buffer = std::shared_ptr<uint8_t[]>(new uint8_t[_current_sector_size]);
        _current_write_block_size = _current_sector_size;
    }
#endif

    // check flash algo every sector change, addresses with different flash algo should be sector aligned
    status = flash_algo_set(_current_sector_addr);
    if (ERR_NONE != status)
    {
        LOG_ERROR("Flash algo set failed");
        flash_uninit();
        return status;
    }

    // Clear out buffer in case block size changed
    memset(_page_buffer.get(), 0xFF, _current_write_block_size);

    return ERR_NONE;
}

FlashIface::err_t FlashAccessor::init(const target_cfg_t &cfg)
{
    FlashIface::err_t status = ERR_NONE;

    if (_flash_state != FLASH_STATE_CLOSED)
    {
        return ERR_INTERNAL;
    }

    // Initialize variables
    _page_buf_empty = true;
    _current_sector_valid = false;
    _current_write_block_addr = 0;
    _current_write_block_size = 0;
    _current_sector_addr = 0;
    _current_sector_size = 0;
    _last_packet_addr = 0;

    // Initialize flash
    status = flash_init(cfg);
    if (ERR_NONE != status)
    {
        LOG_ERROR("Flash init failed");
        return status;
    }

    LOG_INFO("Flash init successful");
    _flash_state = FLASH_STATE_OPEN;

    return status;
}

FlashIface::err_t FlashAccessor::write(uint32_t packet_addr, const uint8_t *data, uint32_t size)
{
    uint32_t page_buf_left = 0;
    uint32_t copy_size = 0;
    uint32_t copy_start_pos = 0;
    FlashIface::err_t status = ERR_NONE;

    if (_flash_state != FLASH_STATE_OPEN)
    {
        return ERR_INTERNAL;
    }

    // Setup the current sector if it is not setup already
    if (!_current_sector_valid)
    {
        status = setup_next_sector(packet_addr);
        if (ERR_NONE != status)
        {
            _flash_state = FLASH_STATE_ERROR;
            return status;
        }

        _current_sector_valid = true;
        _last_packet_addr = packet_addr;
    }

    // non-increasing address support
    if (ROUND_DOWN(packet_addr, _current_write_block_size) != ROUND_DOWN(_last_packet_addr, _current_write_block_size))
    {
        status = flush_current_block(packet_addr);
        if (ERR_NONE != status)
        {
            _flash_state = FLASH_STATE_ERROR;
            return status;
        }
    }

    if (ROUND_DOWN(packet_addr, _current_sector_size) != ROUND_DOWN(_last_packet_addr, _current_sector_size))
    {
        status = setup_next_sector(packet_addr);
        if (ERR_NONE != status)
        {
            _flash_state = FLASH_STATE_ERROR;
            return status;
        }
    }

    while (true)
    {
        // flush if necessary
        if (packet_addr >= _current_write_block_addr + _current_write_block_size)
        {
            status = flush_current_block(packet_addr);

            if (ERR_NONE != status)
            {
                _flash_state = FLASH_STATE_ERROR;
                return status;
            }
        }

        // Check for end
        if (size <= 0)
        {
            break;
        }

        // Change sector if necessary
        if (packet_addr >= _current_sector_addr + _current_sector_size)
        {
            status = setup_next_sector(packet_addr);
            if (ERR_NONE != status)
            {
                _flash_state = FLASH_STATE_ERROR;
                return status;
            }
        }

        // write buffer
        copy_start_pos = packet_addr - _current_write_block_addr;
        page_buf_left = _current_write_block_size - copy_start_pos;
        copy_size = ((size) < (page_buf_left) ? (size) : (page_buf_left));
        memcpy(_page_buffer.get() + copy_start_pos, data, copy_size);
        _page_buf_empty = (copy_size == 0);

        // Update variables
        packet_addr += copy_size;
        data += copy_size;
        size -= copy_size;
    }

    _last_packet_addr = packet_addr;

    return status;
}

FlashIface::err_t FlashAccessor::uninit()
{
    FlashIface::err_t flash_write_ret = ERR_NONE;
    FlashIface::err_t flash_uninit_ret = ERR_NONE;

    if (FLASH_STATE_CLOSED == _flash_state)
    {
        return ERR_INTERNAL;
    }

    // Flush last buffer if its not empty
    if (FLASH_STATE_OPEN == _flash_state)
    {
        flash_write_ret = flush_current_block(0);
    }

    // Close flash interface (even if there was an error during program_page)
    flash_uninit_ret = flash_uninit();

    _page_buf_empty = true;
    _current_sector_valid = false;
    _current_write_block_addr = 0;
    _current_write_block_size = 0;
    _current_sector_addr = 0;
    _current_sector_size = 0;
    _last_packet_addr = 0;
    _flash_state = FLASH_STATE_CLOSED;

    // Make sure an error from a page write or from an uninit gets propagated
    if (flash_uninit_ret != ERR_NONE)
    {
        return flash_uninit_ret;
    }

    if (flash_write_ret != ERR_NONE)
    {
        return flash_write_ret;
    }

    return ERR_NONE;
}