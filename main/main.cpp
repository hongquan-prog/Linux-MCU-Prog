#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>
#include "log.h"
#include "algorithm.h"
#include "flash_iface.h"
#include "fsl_rgpio.h"
#include "file_programmer.h"
#include "bin_program.h"
#include "hex_program.h"
#include <sys/time.h>

#define TAG "main"

static int dev_mem_fd;
static Algorithm algo;
static FlashIface::program_target_t target;
static FlashIface::target_cfg_t cfg;
static BinaryProgram bin_program;
static HexProgram hex_program;
static FileProgrammer prog(bin_program, hex_program);

void terminate(int sig)
{
    munmap((void *)GPIO1, sysconf(_SC_PAGE_SIZE));
    munmap((void *)GPIO2, sysconf(_SC_PAGE_SIZE));
    munmap((void *)GPIO3, sysconf(_SC_PAGE_SIZE));
    munmap((void *)GPIO4, sysconf(_SC_PAGE_SIZE));
    close(dev_mem_fd);
    exit(0);
}

int main(int argc, char *argv[])
{
    struct timeval start = {0};
    struct timeval end = {0};
    uint32_t base_addr = 0;

    if (argc < 3 || argc > 4)
    {
        LOG_ERROR("Usage: %s algorithm image <addr>", argv[0]);
        return -1;
    }
    else if (argc == 4)
    {
        base_addr = static_cast<uint32_t>(std::stoul(argv[3], nullptr, 16));
    }

    signal(SIGINT, terminate);
    dev_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (dev_mem_fd < 0)
    {
        LOG_ERROR("open: %s", strerror(errno));
        return -1;
    }

    LOG_INFO("imx_gpio mmap: pagesize: %u", (unsigned int)sysconf(_SC_PAGE_SIZE));
    GPIO1 = reinterpret_cast<volatile RGPIO_Type *>(mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, GPIO1_BASE));
    GPIO2 = reinterpret_cast<volatile RGPIO_Type *>(mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, GPIO2_BASE));
    GPIO3 = reinterpret_cast<volatile RGPIO_Type *>(mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, GPIO3_BASE));
    GPIO4 = reinterpret_cast<volatile RGPIO_Type *>(mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, GPIO4_BASE));

    if ((GPIO1 == MAP_FAILED) || (GPIO2 == MAP_FAILED) || (GPIO3 == MAP_FAILED) || (GPIO4 == MAP_FAILED))
    {
        LOG_ERROR("mmap: %s", strerror(errno));
        goto _exit;
        return -1;
    }

    try
    {

        if (algo.extract(argv[1], target, cfg))
        {
            LOG_INFO("algorithm extracted from %s", argv[1]);
            gettimeofday(&start, NULL);
            prog.program(argv[2], cfg, base_addr);
            gettimeofday(&end, NULL);
            LOG_INFO("烧录时间：%ld ms", ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / 1000);
        }
    }
    catch (std::exception &e)
    {
        LOG_ERROR("%s", e.what());
    }

_exit:
    terminate(SIGINT);
}