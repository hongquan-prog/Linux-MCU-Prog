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
#include "log.h"
#include "algorithm.h"
#include "flash_iface.h"
#include "file_programmer.h"
#include "bin_program.h"
#include "hex_program.h"
#include "rk3588_swd.h"
#include <sys/time.h>

#define TAG "main"

static Algorithm algo;
static FlashIface::program_target_t target;
static FlashIface::target_cfg_t cfg;

void progress_bar(int progress)
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
        LOG_PRINTF("program progress:: [%s] %d%%\r", buf, progress);
    else
        LOG_PRINTF("program progress:: [%s] %d%%\r\n", buf, progress);

    fflush(stdout);
}

void progress_handler(int progress)
{
    progress_bar(progress);
}

int main(int argc, char *argv[])
{
    struct timeval start = {0};
    struct timeval end = {0};
    uint32_t base_addr = 0;
    static RK3588SWD &swd = RK3588SWD::instance(RK3588_GPIO4, 1, RK3588_GPIO4, 2);
    static BinaryProgram bin_program(swd);
    static HexProgram hex_program(swd);
    static FileProgrammer prog(bin_program, hex_program);

    if (argc < 3 || argc > 4)
    {
        LOG_ERROR("Usage: %s algorithm image <addr>", argv[0]);
        return -1;
    }
    else if (argc == 4)
    {
        base_addr = static_cast<uint32_t>(std::stoul(argv[3], nullptr, 16));
    }

    try
    {
        if (algo.extract(argv[1], target, cfg))
        {
            prog.register_progress_changed_callback(progress_handler);
            LOG_INFO("algorithm extracted from %s", argv[1]);
            gettimeofday(&start, NULL);
            prog.program(argv[2], cfg, base_addr);
            gettimeofday(&end, NULL);
            LOG_INFO("elapsed time: %ld ms", ((end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec)) / 1000);
        }
    }
    catch (std::exception &e)
    {
        LOG_ERROR("%s", e.what());
    }
}