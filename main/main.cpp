#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "log.h"
#include "flash_iface.h"
#include "file_programmer.h"
#include "bin_program.h"
#include "hex_program.h"
#include "algorithm.h"
#include "ah6_swd.h"
#include "bcm2835_swd.h"
#include "sg2002_swd.h"

#define TAG "main"

__attribute__((weak)) const unsigned char *algorithm_bin_ptr()
{
    return nullptr;
}

__attribute__((weak)) unsigned int algorithm_bin_len()
{
    return 0;
}

void progress_handler(int progress)
{
    int len = 0;
    static char buf[200];
    for (int i = 0; i < progress / 5; i++)
    {
        len = len + snprintf(buf + len, sizeof(buf) - len, "-");
    }

    for (int i = progress / 5; i < 100 / 5; i++)
    {
        len = len + snprintf(buf + len, sizeof(buf) - len, " ");
    }

    if (progress != 100)
        LOG_PROGRESS("program progress:: [%s] %d%%", buf, progress);
    else
        LOG_PROGRESS("program progress:: [%s] %d%%\n", buf, progress);

    fflush(stdout);

    for (int i = 0; i < 200; i++)
    {
        printf("\b");
    }
}

std::vector<std::string> split_string(const std::string &str, char seperator)
{
    std::string temp;
    std::vector<std::string> result;

    for (char ch : str)
    {
        if (ch == seperator)
        {
            if (!temp.empty())
            {
                result.push_back(temp);
                temp.clear();
            }
        }
        else
        {
            temp += ch;
        }
    }

    if (!temp.empty())
    {
        result.push_back(temp);
    }

    return result;
}

std::shared_ptr<SWDIface> swd_create(uint32_t swclk, uint32_t swdio)
{
    std::string temp;
    std::shared_ptr<SWDIface> swd;
    std::vector<std::string> compatible;
    std::ifstream file("/proc/device-tree/compatible");

    if (file.is_open())
    {
        if (getline(file, temp))
        {
            LOG_INFO("device %s", temp.c_str());
            compatible = split_string(temp, '\0');

            for (auto name : compatible)
            {
                if (name.compare("allwinner,h616") == 0)
                {
                    swd = std::make_shared<AH618SWD>(swclk, swdio, 2000000, true);
                    break;
                }
                else if (name.compare("allwinner,sun50i-h6") == 0)
                {
                    swd = std::make_shared<AH6SWD>(swclk, swdio, 2000000, true);
                    break;
                }
                else if (name.compare("brcm,bcm2837") == 0)
                {
                    swd = std::make_shared<BCM2835SWD>(swclk, swdio, 5000000, true);
                }
                else if (name.compare("cvitek,cv181x") == 0)
                {
                    swd = std::make_shared<SG2002SWD>(swclk, swdio, 5000000, true);
                }
            }
        }

        file.close();
        if (!swd)
        {
            throw std::runtime_error("Unsupported device types " + temp);
        }
    }
    else
    {
        throw std::runtime_error("Unable to open /proc/device-tree/compatible");
    }

    return swd;
}

bool file_exist(const std::string &path)
{
    std::ifstream file(path);
    return file.is_open();
}

bool read_memory(std::shared_ptr<SWDIface> swd, uint32_t addr, uint32_t length)
{
    uint32_t rd_size = 0;
    uint32_t rd_begin = addr;
    uint32_t rd_end = addr + length;
    std::shared_ptr<uint8_t[]> buf = std::shared_ptr<uint8_t[]>(new uint8_t[32]);

    if (swd->set_target_state(SWDIface::TARGET_HALT) != true)
    {
        return false;
    }

    memset(buf.get(), 0, 32);
    std::cout << "memory content:" << std::endl;
    for (uint32_t i = rd_begin; i < rd_end; i += rd_size)
    {
        rd_size = rd_end - i;
        rd_size = (rd_size > 32) ? (32) : (rd_size);
        if (swd->read_memory(i, buf.get(), rd_size) != true)
        {
            swd->set_target_state(SWDIface::TARGET_RUN);
            return false;
        }

        std::cout << std::hex << std::setw(8) << std::setfill('0') << i << ": ";
        for (uint32_t j = 0; j < rd_size; ++j)
        {
            std::cout << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(buf[j]) << " ";
        }

        std::cout << "\n";
    }

    swd->set_target_state(SWDIface::TARGET_RUN);

    return true;
}

int main(int argc, char *argv[])
{
    std::shared_ptr<SWDIface> swd;
    std::shared_ptr<BinaryProgram> bin;
    std::shared_ptr<HexProgram> hex;
    std::shared_ptr<FileProgrammer> prog;
    struct timeval start = {0};
    struct timeval end = {0};
    uint32_t base_addr = 0;
    uint32_t swclk_pin = 0;
    uint32_t swdio_pin = 0;
    FlashIface::target_cfg_t cfg;
    std::string command;
    std::string algorithm;
    std::string image;
    uint32_t rd_addr;
    uint32_t rd_size;

    if (argc < 3)
    {
        LOG_ERROR("Usage: %s <command> [options]", argv[0]);
        LOG_ERROR("Commands: flash, read");
        return -1;
    }

    command = argv[1];
    if (command == "flash")
    {
        if (argc < 6 || argc > 7)
        {
            LOG_ERROR("Usage: %s flash swclk swdio algorithm image <addr>", argv[0]);
            return -1;
        }
        else if (argc == 7)
        {
            base_addr = static_cast<uint32_t>(std::stoul(argv[6], nullptr, 16));
        }

        swclk_pin = static_cast<uint32_t>(std::stoul(argv[2], nullptr, 10));
        swdio_pin = static_cast<uint32_t>(std::stoul(argv[3], nullptr, 10));
        algorithm = argv[4];
        image = argv[5];

        try
        {
            swd = swd_create(swclk_pin, swdio_pin);
            bin = std::make_shared<BinaryProgram>(*swd);
            hex = std::make_shared<HexProgram>(*swd);
            prog = std::make_shared<FileProgrammer>(*bin, *hex);
            prog->register_progress_changed_callback(progress_handler);

            // Finding Algorithms from Programs
            if (Algorithm::find(algorithm_bin_ptr(), algorithm_bin_len(), algorithm, cfg))
            {
                gettimeofday(&start, NULL);
                if (prog->program(image, cfg, base_addr))
                {
                    gettimeofday(&end, NULL);
                    LOG_INFO("elapsed time: %ld ms", ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / 1000);
                }
            }
            // Finding Algorithms from file
            else if (file_exist(algorithm))
            {
                gettimeofday(&start, NULL);
                if (prog->program(image, algorithm, base_addr))
                {
                    gettimeofday(&end, NULL);
                    LOG_INFO("elapsed time: %ld ms", ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / 1000);
                }
            }
            else
            {
                LOG_ERROR("Unknown device %s", algorithm.c_str());
                Algorithm::list_device(algorithm_bin_ptr(), algorithm_bin_len());
            }
        }
        catch (std::exception &e)
        {
            LOG_ERROR("%s", e.what());
        }
    }
    else if (command == "read")
    {
        if (argc != 6)
        {
            LOG_ERROR("Usage: %s read swclk swdio addr length", argv[0]);
            return -1;
        }

        swclk_pin = static_cast<uint32_t>(std::stoul(argv[2], nullptr, 10));
        swdio_pin = static_cast<uint32_t>(std::stoul(argv[3], nullptr, 10));
        rd_addr = static_cast<uint32_t>(std::stoul(argv[4], nullptr, 16));
        rd_size = static_cast<uint32_t>(std::stoul(argv[5], nullptr, 16));
        swd = swd_create(swclk_pin, swdio_pin);
        read_memory(swd, rd_addr, rd_size);
    }
}