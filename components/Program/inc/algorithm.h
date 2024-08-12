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

#include <cstdint>
#include <string>
#include <vector>
#include <list>
#include <map>
#include "flash_iface.h"

class Algorithm
{
private:

public:
    Algorithm();

    /** @brief Extract the burning algorithm from the elf format algorithm file
     *
     * @param path elf file path
     * @param cfg  Extracted burn configuration
     * @param ram_begin ram base address
     *
     * @return Returns true if the extract succeeds and false if it fails.
     */
    static bool extract(const std::string &path, FlashIface::target_cfg_t &cfg, uint32_t ram_begin = 0x20000000);

    /** @brief Combine multiple algorithm files into a single file for embedding into a program
     *
     * @param path The name of the merged output file
     * @param algo List of Algorithm Files
     *
     * @return Returns true if the merge succeeds and false if it fails.
     */
    static bool merge(const std::string &path, std::list<std::string> &algo);

    /** @brief Finding the burning algorithm from the merged file
     *
     * @param path The name of the merged algorithm file
     * @param name The name of target device
     * @param cfg  Extracted target configuration
     *
     * @return Returns true if found, fails if not.
     */
    static bool find(const std::string &path, const std::string &name, FlashIface::target_cfg_t &cfg);

    /** @brief Finding the burning algorithm from the merged file
     *
     * @param data Algorithms stored in read-only data segments
     * @param len  Length of algorithm data
     * @param name The name of target device
     * @param cfg  Extracted target configuration
     *
     * @return Returns true if found, fails if not.
     */
    static bool find(const unsigned char *data, const unsigned int len, const std::string &name, FlashIface::target_cfg_t &cfg);

    /** @brief List all supported devices
     *
     * @param data Algorithms stored in read-only data segments
     * @param len  Length of algorithm data
     *
     * @return None
     */
    static void list_device(const unsigned char *data, const unsigned int len);
};