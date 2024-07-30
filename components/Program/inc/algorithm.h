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
#include <map>
#include "elf.h"
#include "flash_iface.h"
#include "FlashOS.h"

class Algorithm
{
private:
    typedef struct
    {
        uint32_t offset;
        uint32_t size;
    } elf_func_t;

    static const std::vector<std::string> _function_list;
    static const uint32_t _flash_bolb_header[8];
    static constexpr uint32_t _stack_size = 0x800;

    bool read_string(FILE *fp, Elf_Shdr &str_tab_hdr, uint32_t offset, std::string &str);
    bool find_scn_hdr_by_phdr(FILE *fp, Elf_Ehdr &elf_hdr, Elf_Phdr &phdr, std::vector<Elf_Shdr> &shdr);
    bool get_shstr_hdr(FILE *fp, Elf_Ehdr &elf_hdr, Elf_Shdr &str_tab_hdr);
    bool extract_flash_device(FILE *fp, Elf_Sym &sym, Elf_Shdr &shdr, FlashDevice &dev);
    bool extract_flash_algo(FILE *fp, Elf_Shdr &code_scn, FlashIface::program_target_t &target);
    bool find_shdr(FILE *fp, Elf_Ehdr &elf_hdr, Elf_Shdr &shstr_shdr, const std::string &scn_name, Elf_Shdr &shdr);
    bool find_shdr(FILE *fp, Elf_Ehdr &elf_hdr, Elf_Shdr &shstr_shdr, const std::string &scn_name, uint32_t type, Elf_Shdr &shdr);

    bool read_elf_hdr(FILE *fp, Elf_Ehdr &elf_hdr);
    void read_symbol_info(FILE *fp, Elf_Shdr &string_hdr, Elf_Shdr &sym_hdr, Elf_Shdr &str_hdr, std::map<std::string, Elf_Sym> &func);

public:
    Algorithm();
    bool extract(const std::string &path, FlashIface::program_target_t &target, FlashIface::target_cfg_t &cfg, uint32_t ram_begin = 0x20000000);
};