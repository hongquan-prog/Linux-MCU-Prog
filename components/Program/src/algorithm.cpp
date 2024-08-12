/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-9-8      lihongquan   add license declaration
 */
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <memory>

#include "log.h"
#include "elf.h"
#include "FlashOS.h"
#include "algorithm.h"

#define TAG "algorithm"

typedef enum
{
    TLV_ALGO_HDR,
    TLV_ALGO_INFO,
    TLV_SECTOR_INFO,
    TLV_FLASH_REGION,
    TLV_RAM_REGION
} tlv_magic_def;

typedef struct
{
    uint16_t it_magic;
    uint16_t it_tlv_tot;
} tlv_info_t;

typedef struct
{
    uint32_t start;
    uint32_t end;
    uint32_t flags;
    uint32_t init;
    uint32_t uninit;
    uint32_t erase_chip;
    uint32_t erase_sector;
    uint32_t program_page;
    uint32_t verify;
    SWDIface::syscall_t sys_call_s;
    uint32_t program_buffer;
    uint32_t program_buffer_size;
    uint32_t algo_start;
    uint32_t algo_size;
} disk_region_info_t;

typedef struct
{
    uint8_t erase_reset;
    uint32_t device_name_length;
} algorithm_info_t;

const std::vector<std::string> _function_list = {
    "FlashDevice",
    "Init",
    "UnInit",
    "BlankCheck",
    "EraseChip",
    "EraseSector",
    "ProgramPage",
    "Verify"};

static const uint32_t _flash_bolb_header[8] = {0xE00ABE00, 0x062D780D, 0x24084068, 0xD3000040, 0x1E644058, 0x1C49D1FA, 0x2A001E52, 0x4770D1F};
static constexpr uint32_t CONFIG_ALGORITHM_STACK_SIZE = 0x800;

Algorithm::Algorithm()
{
}

static bool read_elf_hdr(FILE *fp, Elf_Ehdr &elf_hdr)
{
    fseek(fp, 0, SEEK_SET);

    if ((sizeof(Elf_Ehdr) != fread(&elf_hdr, 1, sizeof(Elf_Ehdr), fp)) || !IS_ELF(elf_hdr) || (elf_hdr.e_type != ET_EXEC))
        throw std::runtime_error("Invalid ELF file");

    return true;
}

static bool read_string(FILE *fp, Elf_Shdr &str_shdr, uint32_t offset, std::string &str)
{
    int tmp = 0;

    str.clear();
    fseek(fp, str_shdr.sh_offset + offset, SEEK_SET);
    while ((tmp = fgetc(fp)) != EOF && (tmp != '\0'))
    {
        str.push_back(static_cast<char>(tmp));
    }

    return true;
}

static bool find_shdr(FILE *fp, Elf_Ehdr &elf_hdr, Elf_Shdr &shstr_shdr, const std::string &scn_name, Elf_Shdr &shdr)
{
    long cur_pos = 0;
    std::string name;

    fseek(fp, elf_hdr.e_shoff, SEEK_SET);

    for (int i = 0; i < elf_hdr.e_shnum; i++)
    {
        if ((sizeof(Elf_Shdr) != fread(&shdr, 1, sizeof(Elf_Shdr), fp)))
            throw std::runtime_error("could not find section " + scn_name);

        cur_pos = ftell(fp);

        if (read_string(fp, shstr_shdr, shdr.sh_name, name) && !scn_name.compare(name))
            return true;

        fseek(fp, cur_pos, SEEK_SET);
    }

    throw std::runtime_error("could not find section " + scn_name);
}

static bool get_shstr_hdr(FILE *fp, Elf_Ehdr &elf_hdr, Elf_Shdr &shstr_shdr)
{
    fseek(fp, elf_hdr.e_shoff + elf_hdr.e_shstrndx * elf_hdr.e_shentsize, SEEK_SET);

    if (fread(&shstr_shdr, 1, sizeof(Elf_Shdr), fp) != sizeof(Elf_Shdr))
        throw std::runtime_error("could not find section header table");

    return true;
}

static void read_symbol_info(FILE *fp, Elf_Shdr &string_hdr, Elf_Shdr &sym_hdr, Elf_Shdr &str_hdr, std::map<std::string, Elf_Sym> &sym)
{
    std::string name;
    long cur_pos = 0;
    Elf_Sym tmp = {0, 0, 0, 0, 0, 0};
    int sym_section_num = sym_hdr.sh_size / sizeof(Elf_Sym);

    fseek(fp, sym_hdr.sh_offset, SEEK_SET);

    for (int i = 0; i < sym_section_num; i++)
    {
        if (fread(&tmp, 1, sizeof(Elf_Sym), fp) != sizeof(Elf_Sym))
        {
            throw std::runtime_error("could not symbol info");
        }

        cur_pos = ftell(fp);
        if (read_string(fp, str_hdr, tmp.st_name, name))
        {
            auto it = std::find(_function_list.begin(), _function_list.end(), name);
            if (it != _function_list.end())
            {
                sym[name] = tmp;
            }
        }

        fseek(fp, cur_pos, SEEK_SET);
    }
}

static bool extract_flash_device(FILE *fp, Elf_Sym &sym, Elf_Shdr &shdr, FlashDevice &dev)
{
    fseek(fp, sym.st_value - shdr.sh_addr + shdr.sh_offset, SEEK_SET);

    if (fread(&dev, 1, sizeof(FlashDevice), fp) != sizeof(FlashDevice))
        throw std::runtime_error("could not read flash device info");

    return true;
}

bool extract_flash_algo(FILE *fp, Elf_Shdr &code_scn, std::shared_ptr<FlashIface::program_target_t> target)
{
    fseek(fp, code_scn.sh_offset, SEEK_SET);
    target->algo_size = sizeof(_flash_bolb_header) + code_scn.sh_size;
    target->algo_blob.reset(new uint32_t[target->algo_size / sizeof(uint32_t)]);
    memcpy(target->algo_blob.get(), _flash_bolb_header, sizeof(_flash_bolb_header));

    if (fread(target->algo_blob.get() + sizeof(_flash_bolb_header) / sizeof(uint32_t), 1, code_scn.sh_size, fp) != code_scn.sh_size)
        throw std::runtime_error("could not read flash algo");

    return true;
}

bool Algorithm::extract(const std::string &path, FlashIface::target_cfg_t &cfg, uint32_t ram_start)
{
    bool ret = false;
    FILE *fp = nullptr;
    Elf_Ehdr elf_hdr;
    Elf_Shdr sym_shdr;
    Elf_Shdr str_shdr;
    Elf_Shdr shstr_shdr;
    std::map<std::string, Elf_Sym> sym_table;
    std::map<std::string, Elf_Shdr> prog_table;
    std::unique_ptr<FlashDevice> device = std::make_unique<FlashDevice>();
    std::shared_ptr<FlashIface::program_target_t> target = std::make_shared<FlashIface::program_target_t>();

    try
    {
        memset(&shstr_shdr, 0, sizeof(Elf_Shdr));
        memset(&sym_shdr, 0, sizeof(Elf_Shdr));
        memset(&str_shdr, 0, sizeof(Elf_Shdr));

        fp = fopen(path.c_str(), "r");
        if (fp == nullptr)
            throw std::ifstream::failure("open file failed: " + path);

        read_elf_hdr(fp, elf_hdr);
        get_shstr_hdr(fp, elf_hdr, shstr_shdr);
        find_shdr(fp, elf_hdr, shstr_shdr, ".strtab", str_shdr);
        find_shdr(fp, elf_hdr, shstr_shdr, ".symtab", sym_shdr);
        find_shdr(fp, elf_hdr, shstr_shdr, "DevDscr", prog_table["DevDscr"]);
        find_shdr(fp, elf_hdr, shstr_shdr, "PrgCode", prog_table["PrgCode"]);
        find_shdr(fp, elf_hdr, shstr_shdr, "PrgData", prog_table["PrgData"]);
        read_symbol_info(fp, str_shdr, sym_shdr, str_shdr, sym_table);
        extract_flash_device(fp, sym_table["FlashDevice"], prog_table["DevDscr"], *device);
        extract_flash_algo(fp, prog_table["PrgCode"], target);
        target->algo_start = ram_start;

        /* 在 DAPLink 中，static_base 是指程序的静态基地址（Static Base Address）。静态基地址是一个指针，指向程序的全局变量和静态变量的起始位置。 */
        target->sys_call_s.static_base = target->algo_start + prog_table["PrgCode"].sh_size + sizeof(_flash_bolb_header);
        target->sys_call_s.breakpoint = target->algo_start + 1;
        /* 设置栈顶指针,位于数据段之后,栈大小为4K */
        target->sys_call_s.stack_pointer = ((target->algo_start + target->algo_size) + 0x100 - 1) / 0x100 * 0x100 + CONFIG_ALGORITHM_STACK_SIZE;
        /* 设置烧录数据的首地址，确保首地址为0x100的整数倍 */
        target->program_buffer = target->sys_call_s.stack_pointer;
        target->program_buffer_size = device->szPage;
        /* 设置Flash读写函数的地址 */
        target->init = (sym_table.find("Init") != sym_table.end()) ? (sym_table["Init"].st_value + sizeof(_flash_bolb_header) + target->algo_start) : (0);
        target->uninit = (sym_table.find("UnInit") != sym_table.end()) ? (sym_table["UnInit"].st_value + sizeof(_flash_bolb_header) + target->algo_start) : (0);
        target->erase_chip = (sym_table.find("EraseChip") != sym_table.end()) ? (sym_table["EraseChip"].st_value + sizeof(_flash_bolb_header) + target->algo_start) : (0);
        target->erase_sector = (sym_table.find("EraseSector") != sym_table.end()) ? (sym_table["EraseSector"].st_value + sizeof(_flash_bolb_header) + target->algo_start) : (0);
        target->program_page = (sym_table.find("ProgramPage") != sym_table.end()) ? (sym_table["ProgramPage"].st_value + sizeof(_flash_bolb_header) + target->algo_start) : (0);
        target->verify = (sym_table.find("Verify") != sym_table.end()) ? (sym_table["Verify"].st_value + sizeof(_flash_bolb_header) + target->algo_start) : (0);

        cfg.sector_info.clear();
        cfg.flash_regions.clear();
        cfg.ram_regions.clear();
        cfg.erase_reset = false;
        cfg.flash_regions.push_back(FlashIface::region_info_t{device->devAdr, device->devAdr + device->szDev, FlashIface::REIGION_DEFAULT, target});
        cfg.ram_regions.push_back(FlashIface::region_info_t{ram_start, target->program_buffer + target->program_buffer_size, 0, nullptr});
        cfg.device_name.replace(0, cfg.device_name.size(), device->devName);

        for (int i = 0; i < SECTOR_NUM; i++)
        {
            if (device->sectors[i].adrSector == 0xffffffff && device->sectors[i].szSector == 0xffffffff)
                break;
            cfg.sector_info.push_back(FlashIface::sector_info_t{device->devAdr + device->sectors[i].adrSector, device->sectors[i].szSector});
        }

        ret = true;
    }
    catch (std::exception &e)
    {
        LOG_ERROR("%s", e.what());

        if (target->algo_blob)
        {
            target->algo_blob = nullptr;
        }
    }

    if (fp)
        fclose(fp);

    return ret;
}

static void algorithm_info_write(std::ofstream &file, FlashIface::target_cfg_t &cfg)
{
    algorithm_info_t algo_info;
    tlv_info_t tlv = {TLV_ALGO_HDR, 0};

    // algorithm tlv header
    tlv.it_tlv_tot += sizeof(tlv_info_t);
    // algorithm info tlv header
    tlv.it_tlv_tot += sizeof(tlv_info_t);
    // algorithm info
    tlv.it_tlv_tot += sizeof(algorithm_info_t);
    tlv.it_tlv_tot += cfg.device_name.length() + 1;

    // sector info
    for (auto info : cfg.sector_info)
    {
        tlv.it_tlv_tot = tlv.it_tlv_tot + sizeof(tlv_info_t) + sizeof(info);
    }

    // flash regions
    for (auto info : cfg.flash_regions)
    {
        tlv.it_tlv_tot += sizeof(tlv_info_t);
        tlv.it_tlv_tot += sizeof(disk_region_info_t);
        if (info.flash_algo)
        {
            tlv.it_tlv_tot += info.flash_algo->algo_size;
        }
    }

    // ram regions
    for (auto info : cfg.ram_regions)
    {
        tlv.it_tlv_tot += sizeof(tlv_info_t);
        tlv.it_tlv_tot += sizeof(disk_region_info_t);
        if (info.flash_algo)
        {
            tlv.it_tlv_tot += info.flash_algo->algo_size;
        }
    }

    // algorithm header tlv
    file.write(reinterpret_cast<char *>(&tlv), sizeof(tlv));
    // algorithm info tlv
    tlv.it_magic = TLV_ALGO_INFO;
    tlv.it_tlv_tot = sizeof(algo_info) + cfg.device_name.length() + 1;
    file.write(reinterpret_cast<char *>(&tlv), sizeof(tlv));
    // algorithm info
    algo_info.erase_reset = cfg.erase_reset;
    algo_info.device_name_length = cfg.device_name.length() + 1;
    file.write(reinterpret_cast<char *>(&algo_info), sizeof(algo_info));
    file.write(cfg.device_name.c_str(), cfg.device_name.length() + 1);
}

static void region_info_write(std::ofstream &file, FlashIface::region_info_t &region_info, tlv_magic_def magic)
{
    tlv_info_t tlv = {magic, 0};
    disk_region_info_t disk_region = {0};

    // region info tlv
    tlv.it_tlv_tot = sizeof(disk_region);
    if (region_info.flash_algo && region_info.flash_algo->algo_blob)
    {
        tlv.it_tlv_tot += region_info.flash_algo->algo_size;
    }

    file.write(reinterpret_cast<char *>(&tlv), sizeof(tlv));

    // region info
    disk_region.start = region_info.start;
    disk_region.end = region_info.end;
    disk_region.flags = region_info.flags;
    if (region_info.flash_algo)
    {
        disk_region.init = region_info.flash_algo->init;
        disk_region.uninit = region_info.flash_algo->uninit;
        disk_region.erase_chip = region_info.flash_algo->erase_chip;
        disk_region.erase_sector = region_info.flash_algo->erase_sector;
        disk_region.program_page = region_info.flash_algo->program_page;
        disk_region.verify = region_info.flash_algo->verify;
        disk_region.sys_call_s = region_info.flash_algo->sys_call_s;
        disk_region.program_buffer = region_info.flash_algo->program_buffer;
        disk_region.program_buffer_size = region_info.flash_algo->program_buffer_size;
        disk_region.algo_start = region_info.flash_algo->algo_start;
        disk_region.algo_size = region_info.flash_algo->algo_size;
    }

    file.write(reinterpret_cast<char *>(&disk_region), sizeof(disk_region));
    if (region_info.flash_algo && region_info.flash_algo->algo_blob)
    {
        file.write(reinterpret_cast<char *>(region_info.flash_algo->algo_blob.get()), disk_region.algo_size);
    }
}

static void sector_info_write(std::ofstream &file, FlashIface::sector_info_t &info)
{
    tlv_info_t tlv = {TLV_SECTOR_INFO, sizeof(FlashIface::sector_info_t)};

    file.write(reinterpret_cast<char *>(&tlv), sizeof(tlv));
    file.write(reinterpret_cast<char *>(&info), sizeof(info));
}

static void append(std::ofstream &file, FlashIface::target_cfg_t &cfg)
{
    algorithm_info_write(file, cfg);
    for (auto info : cfg.sector_info)
    {
        sector_info_write(file, info);
    }

    for (auto info : cfg.flash_regions)
    {
        region_info_write(file, info, TLV_FLASH_REGION);
    }

    for (auto info : cfg.ram_regions)
    {
        region_info_write(file, info, TLV_RAM_REGION);
    }
}

bool Algorithm::merge(const std::string &path, std::list<std::string> &algo)
{
    FlashIface::target_cfg_t cfg;
    std::ofstream file(path, std::ios::binary);

    for (auto item : algo)
    {
        if (extract(item, cfg))
        {
            append(file, cfg);
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool Algorithm::find(const std::string &path, const std::string &name, FlashIface::target_cfg_t &cfg)
{
    bool ret = false;
    size_t hdr_pos = 0;
    tlv_info_t tlv = {0};
    algorithm_info_t algo_info = {0};
    disk_region_info_t disk_region = {0};
    std::shared_ptr<char[]> dev = nullptr;
    std::ifstream file(path, std::ios::binary);
    FlashIface::sector_info_t sector_info = {0};
    FlashIface::region_info_t region_info = {0};
    std::shared_ptr<FlashIface::program_target_t> target = nullptr;

    if (file.is_open() != true)
    {
        return false;
    }

    for (; !file.eof();)
    {
        file.seekg(hdr_pos);
        file.read(reinterpret_cast<char *>(&tlv), sizeof(tlv));
        if (!file.eof() && (tlv.it_magic == TLV_ALGO_HDR))
        {
            hdr_pos += tlv.it_tlv_tot;
            file.read(reinterpret_cast<char *>(&tlv), sizeof(tlv));
            if (tlv.it_magic == TLV_ALGO_INFO)
            {
                file.read(reinterpret_cast<char *>(&algo_info), sizeof(algo_info));
                dev.reset(new char[algo_info.device_name_length]);
                file.read(dev.get(), algo_info.device_name_length);
                if (name.compare(dev.get()) == 0)
                {
                    ret = true;
                    cfg.device_name = name;
                    cfg.erase_reset = algo_info.erase_reset;
                    cfg.flash_regions.clear();
                    cfg.ram_regions.clear();
                    cfg.sector_info.clear();

                    for (;;)
                    {
                        file.read(reinterpret_cast<char *>(&tlv), sizeof(tlv));
                        if (tlv.it_magic == TLV_ALGO_HDR)
                        {
                            break;
                        }
                        else if (tlv.it_magic == TLV_SECTOR_INFO)
                        {
                            file.read(reinterpret_cast<char *>(&sector_info), sizeof(sector_info));
                            cfg.sector_info.push_back(sector_info);
                        }
                        else if ((tlv.it_magic == TLV_FLASH_REGION) || (tlv.it_magic == TLV_RAM_REGION))
                        {
                            target = std::make_shared<FlashIface::program_target_t>();
                            file.read(reinterpret_cast<char *>(&disk_region), sizeof(disk_region));
                            region_info.flash_algo = target;
                            region_info.start = disk_region.start;
                            region_info.end = disk_region.end;
                            region_info.flags = disk_region.flags;
                            target->init = disk_region.init;
                            target->uninit = disk_region.uninit;
                            target->erase_chip = disk_region.erase_chip;
                            target->erase_sector = disk_region.erase_sector;
                            target->program_page = disk_region.program_page;
                            target->verify = disk_region.verify;
                            target->sys_call_s = disk_region.sys_call_s;
                            target->program_buffer = disk_region.program_buffer;
                            target->program_buffer_size = disk_region.program_buffer_size;
                            target->algo_start = disk_region.algo_start;
                            target->algo_size = disk_region.algo_size;
                            if (disk_region.algo_size)
                            {
                                target->algo_blob.reset(new uint32_t[disk_region.algo_size / sizeof(uint32_t)]);
                                file.read(reinterpret_cast<char *>(target->algo_blob.get()), disk_region.algo_size);
                            }

                            if (tlv.it_magic == TLV_FLASH_REGION)
                            {
                                cfg.flash_regions.push_back(region_info);
                            }
                            else
                            {
                                cfg.ram_regions.push_back(region_info);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

bool Algorithm::find(const unsigned char *data, const unsigned int len, const std::string &name, FlashIface::target_cfg_t &cfg)
{
    bool ret = false;
    size_t hdr_pos = 0;
    size_t read_pos = 0;
    tlv_info_t tlv = {0};
    algorithm_info_t algo_info = {0};
    disk_region_info_t disk_region = {0};
    std::shared_ptr<char[]> dev = nullptr;
    FlashIface::sector_info_t sector_info = {0};
    FlashIface::region_info_t region_info = {0};
    std::shared_ptr<FlashIface::program_target_t> target = nullptr;

    if (!data || (len == 0))
    {
        return false;
    }

    for (; (read_pos < len);)
    {
        read_pos = hdr_pos;
        memcpy(&tlv, data + read_pos, sizeof(tlv));
        read_pos += sizeof(tlv);
        if ((read_pos < len) && (tlv.it_magic == TLV_ALGO_HDR))
        {
            hdr_pos += tlv.it_tlv_tot;
            memcpy(&tlv, data + read_pos, sizeof(tlv));
            read_pos += sizeof(tlv);
            if (tlv.it_magic == TLV_ALGO_INFO)
            {
                memcpy(&algo_info, data + read_pos, sizeof(algo_info));
                read_pos += sizeof(algo_info);
                dev.reset(new char[algo_info.device_name_length]);
                memcpy(dev.get(), data + read_pos, algo_info.device_name_length);
                read_pos += algo_info.device_name_length;
                if (name.compare(dev.get()) == 0)
                {
                    ret = true;
                    cfg.device_name = name;
                    cfg.erase_reset = algo_info.erase_reset;
                    cfg.flash_regions.clear();
                    cfg.ram_regions.clear();
                    cfg.sector_info.clear();

                    for (;;)
                    {
                        memcpy(&tlv, data + read_pos, sizeof(tlv));
                        read_pos += sizeof(tlv);
                        if (tlv.it_magic == TLV_ALGO_HDR)
                        {
                            break;
                        }
                        else if (tlv.it_magic == TLV_SECTOR_INFO)
                        {
                            memcpy(&sector_info, data + read_pos, sizeof(sector_info));
                            read_pos += sizeof(sector_info);
                            cfg.sector_info.push_back(sector_info);
                        }
                        else if ((tlv.it_magic == TLV_FLASH_REGION) || (tlv.it_magic == TLV_RAM_REGION))
                        {
                            target = std::make_shared<FlashIface::program_target_t>();
                            memcpy(&disk_region, data + read_pos, sizeof(disk_region));
                            read_pos += sizeof(disk_region);
                            region_info.flash_algo = target;
                            region_info.start = disk_region.start;
                            region_info.end = disk_region.end;
                            region_info.flags = disk_region.flags;
                            target->init = disk_region.init;
                            target->uninit = disk_region.uninit;
                            target->erase_chip = disk_region.erase_chip;
                            target->erase_sector = disk_region.erase_sector;
                            target->program_page = disk_region.program_page;
                            target->verify = disk_region.verify;
                            target->sys_call_s = disk_region.sys_call_s;
                            target->program_buffer = disk_region.program_buffer;
                            target->program_buffer_size = disk_region.program_buffer_size;
                            target->algo_start = disk_region.algo_start;
                            target->algo_size = disk_region.algo_size;
                            if (disk_region.algo_size)
                            {
                                target->algo_blob.reset(new uint32_t[disk_region.algo_size / sizeof(uint32_t)]);
                                memcpy(target->algo_blob.get(), data + read_pos, disk_region.algo_size);
                                read_pos += disk_region.algo_size;
                            }

                            if (tlv.it_magic == TLV_FLASH_REGION)
                            {
                                cfg.flash_regions.push_back(region_info);
                            }
                            else
                            {
                                cfg.ram_regions.push_back(region_info);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

void Algorithm::list_device(const unsigned char *data, const unsigned int len)
{
    int index = 0;
    size_t hdr_pos = 0;
    size_t read_pos = 0;
    tlv_info_t tlv = {0};
    algorithm_info_t algo_info = {0};
    std::shared_ptr<char[]> dev = nullptr;

    if (!data || (len == 0))
    {
        return;
    }

    std::cout << "Available devices:" << std::endl;
    for (; (read_pos < len);)
    {
        read_pos = hdr_pos;
        memcpy(&tlv, data + read_pos, sizeof(tlv));
        read_pos += sizeof(tlv);
        if ((read_pos < len) && (tlv.it_magic == TLV_ALGO_HDR))
        {
            hdr_pos += tlv.it_tlv_tot;
            memcpy(&tlv, data + read_pos, sizeof(tlv));
            read_pos += sizeof(tlv);
            if (tlv.it_magic == TLV_ALGO_INFO)
            {
                memcpy(&algo_info, data + read_pos, sizeof(algo_info));
                read_pos += sizeof(algo_info);
                dev.reset(new char[algo_info.device_name_length]);
                memcpy(dev.get(), data + read_pos, algo_info.device_name_length);
                read_pos += algo_info.device_name_length;
                std::cout << ++index << ". " << dev.get() << std::endl;
            }
        }
    }
}