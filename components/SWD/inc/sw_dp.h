#pragma once

#include "compiler.h"

#define SWD_DATA_PHASE 0
#define SWD_TURNAROUND 1
#define SWD_IDLE_CYCLES 0

typedef struct
{
    void *base;
    int pin;
} swd_pin_t;

#define SWD_IO_OPERATIONS_DEFINE(INIT, SET, CLR, READ, WRITE)                        \
                                                                                     \
    __STATIC_INLINE void PORT_INIT(volatile swd_pin_t *clk, volatile swd_pin_t *dio) \
    {                                                                                \
        INIT(dio->base, dio->pin, true);                                             \
        INIT(clk->base, clk->pin, true);                                             \
        SET(dio->base, dio->pin);                                                    \
        SET(clk->base, clk->pin);                                                    \
        CLR(dio->base, dio->pin);                                                    \
        SET(clk->base, clk->pin);                                                    \
        LOG_INFO("SWD GPIO init done");                                              \
    }                                                                                \
                                                                                     \
    __STATIC_INLINE void PORT_OFF(volatile swd_pin_t *clk, volatile swd_pin_t *dio)  \
    {                                                                                \
        INIT(dio->base, dio->pin, false);                                            \
        INIT(clk->base, clk->pin, false);                                            \
        LOG_INFO("SWD GPIO Reset done");                                             \
    }                                                                                \
                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWCLK_SET(volatile swd_pin_t *io)                  \
    {                                                                                \
        SET(io->base, io->pin);                                                      \
    }                                                                                \
                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWCLK_CLR(volatile swd_pin_t *io)                  \
    {                                                                                \
        CLR(io->base, io->pin);                                                      \
    }                                                                                \
                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_TMS_SET(volatile swd_pin_t *io)              \
    {                                                                                \
        SET(io->base, io->pin);                                                      \
    }                                                                                \
                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_TMS_CLR(volatile swd_pin_t *io)              \
    {                                                                                \
        CLR(io->base, io->pin);                                                      \
    }                                                                                \
                                                                                     \
    __STATIC_FORCEINLINE uint32_t PIN_SWDIO_IN(volatile swd_pin_t *io)               \
    {                                                                                \
        return READ(io->base, io->pin);                                              \
    }                                                                                \
                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_OUT(volatile swd_pin_t *io, uint32_t bit)    \
    {                                                                                \
        WRITE(io->base, io->pin, bit & 0x1);                                         \
    }                                                                                \
                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_OUT_ENABLE(volatile swd_pin_t *io)           \
    {                                                                                \
        INIT(io->base, io->pin, true);                                               \
    }                                                                                \
                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_OUT_DISABLE(volatile swd_pin_t *io)          \
    {                                                                                \
        INIT(io->base, io->pin, false);                                              \
    }                                                                                \
                                                                                     \
    __STATIC_FORCEINLINE void PIN_nRESET_OUT(volatile swd_pin_t *io, uint32_t bit)   \
    {                                                                                \
    }

__STATIC_FORCEINLINE void PIN_DELAY(volatile uint32_t delay)
{
    while (delay)
    {
        --delay;
    }
}

#define SW_CLOCK_CYCLE(CLK, DELAY) \
    PIN_SWCLK_CLR(CLK);            \
    PIN_DELAY(DELAY);              \
    PIN_SWCLK_SET(CLK);            \
    PIN_DELAY(DELAY);

#define SW_WRITE_BIT(CLK, DIO, DELAY, BIT) \
    PIN_SWDIO_OUT(DIO, BIT);               \
    PIN_SWCLK_CLR(CLK);                    \
    PIN_DELAY(DELAY);                      \
    PIN_SWCLK_SET(CLK);                    \
    PIN_DELAY(DELAY);

#define SW_READ_BIT(CLK, DIO, DELAY, BIT) \
    PIN_SWCLK_CLR(CLK);                   \
    PIN_DELAY(DELAY);                     \
    bit = PIN_SWDIO_IN(DIO);              \
    PIN_SWCLK_SET(CLK);                   \
    PIN_DELAY(DELAY);

#define SWJ_SEQUENCE_DEFINE(name)                                                                                           \
    static void name(volatile swd_pin_t *clk, volatile swd_pin_t *dio, uint32_t delay, uint32_t count, const uint8_t *data) \
    {                                                                                                                       \
        uint32_t val;                                                                                                       \
        uint32_t n;                                                                                                         \
                                                                                                                            \
        val = 0U;                                                                                                           \
        n = 0U;                                                                                                             \
        while (count--)                                                                                                     \
        {                                                                                                                   \
            if (n == 0U)                                                                                                    \
            {                                                                                                               \
                val = *data++;                                                                                              \
                n = 8U;                                                                                                     \
            }                                                                                                               \
            if (val & 1U)                                                                                                   \
            {                                                                                                               \
                PIN_SWDIO_TMS_SET(dio);                                                                                     \
            }                                                                                                               \
            else                                                                                                            \
            {                                                                                                               \
                PIN_SWDIO_TMS_CLR(dio);                                                                                     \
            }                                                                                                               \
            SW_CLOCK_CYCLE(clk, delay);                                                                                     \
            val >>= 1;                                                                                                      \
            n--;                                                                                                            \
        }                                                                                                                   \
    }

#define SWD_TRANSFER_DEFINE(name)                                                                                    \
    uint8_t name(volatile swd_pin_t *clk, volatile swd_pin_t *dio, uint32_t delay, uint32_t request, uint32_t *data) \
    {                                                                                                                \
        uint32_t ack;                                                                                                \
        uint32_t bit;                                                                                                \
        uint32_t val;                                                                                                \
        uint32_t parity;                                                                                             \
                                                                                                                     \
        uint32_t n;                                                                                                  \
                                                                                                                     \
        /* Packet Request */                                                                                         \
        parity = 0U;                                                                                                 \
        SW_WRITE_BIT(clk, dio, delay, 1U); /* Start Bit */                                                           \
        bit = request >> 0;                                                                                          \
        SW_WRITE_BIT(clk, dio, delay, bit); /* APnDP Bit */                                                          \
        parity += bit;                                                                                               \
        bit = request >> 1;                                                                                          \
        SW_WRITE_BIT(clk, dio, delay, bit); /* RnW Bit */                                                            \
        parity += bit;                                                                                               \
        bit = request >> 2;                                                                                          \
        SW_WRITE_BIT(clk, dio, delay, bit); /* A2 Bit */                                                             \
        parity += bit;                                                                                               \
        bit = request >> 3;                                                                                          \
        SW_WRITE_BIT(clk, dio, delay, bit); /* A3 Bit */                                                             \
        parity += bit;                                                                                               \
        SW_WRITE_BIT(clk, dio, delay, parity); /* Parity Bit */                                                      \
        SW_WRITE_BIT(clk, dio, delay, 0U);     /* Stop Bit */                                                        \
        SW_WRITE_BIT(clk, dio, delay, 1U);     /* Park Bit */                                                        \
                                                                                                                     \
        /* Turnaround */                                                                                             \
        PIN_SWDIO_OUT_DISABLE(dio);                                                                                  \
        for (n = SWD_TURNAROUND; n; n--)                                                                             \
        {                                                                                                            \
            SW_CLOCK_CYCLE(clk, delay);                                                                              \
        }                                                                                                            \
                                                                                                                     \
        /* Acknowledge response */                                                                                   \
        SW_READ_BIT(clk, dio, delay, bit);                                                                           \
        ack = bit << 0;                                                                                              \
        SW_READ_BIT(clk, dio, delay, bit);                                                                           \
        ack |= bit << 1;                                                                                             \
        SW_READ_BIT(clk, dio, delay, bit);                                                                           \
        ack |= bit << 2;                                                                                             \
        /* OK response */                                                                                            \
        if (ack == DAP_TRANSFER_OK)                                                                                  \
        {                                                                                                            \
            /* Data transfer */                                                                                      \
            if (request & DAP_TRANSFER_RnW)                                                                          \
            {                                                                                                        \
                /* Read data */                                                                                      \
                val = 0U;                                                                                            \
                parity = 0U;                                                                                         \
                for (n = 32U; n; n--)                                                                                \
                {                                                                                                    \
                    SW_READ_BIT(clk, dio, delay, bit); /* Read RDATA[0:31] */                                        \
                    parity += bit;                                                                                   \
                    val >>= 1;                                                                                       \
                    val |= bit << 31;                                                                                \
                }                                                                                                    \
                SW_READ_BIT(clk, dio, delay, bit); /* Read Parity */                                                 \
                if ((parity ^ bit) & 1U)                                                                             \
                {                                                                                                    \
                    ack = DAP_TRANSFER_ERROR;                                                                        \
                }                                                                                                    \
                if (data)                                                                                            \
                {                                                                                                    \
                    *data = val;                                                                                     \
                }                                                                                                    \
                /* Turnaround */                                                                                     \
                for (n = SWD_TURNAROUND; n; n--)                                                                     \
                {                                                                                                    \
                    SW_CLOCK_CYCLE(clk, delay);                                                                      \
                }                                                                                                    \
                PIN_SWDIO_OUT_ENABLE(dio);                                                                           \
            }                                                                                                        \
            else                                                                                                     \
            {                                                                                                        \
                /* Turnaround */                                                                                     \
                for (n = SWD_TURNAROUND; n; n--)                                                                     \
                {                                                                                                    \
                    SW_CLOCK_CYCLE(clk, delay);                                                                      \
                }                                                                                                    \
                PIN_SWDIO_OUT_ENABLE(dio);                                                                           \
                /* Write data */                                                                                     \
                val = *data;                                                                                         \
                parity = 0U;                                                                                         \
                for (n = 32U; n; n--)                                                                                \
                {                                                                                                    \
                    SW_WRITE_BIT(clk, dio, delay, val); /* Write WDATA[0:31] */                                      \
                    parity += val;                                                                                   \
                    val >>= 1;                                                                                       \
                }                                                                                                    \
                SW_WRITE_BIT(clk, dio, delay, parity); /* Write Parity Bit */                                        \
            }                                                                                                        \
            /* Idle cycles */                                                                                        \
            n = SWD_IDLE_CYCLES;                                                                                     \
            if (n)                                                                                                   \
            {                                                                                                        \
                PIN_SWDIO_OUT(dio, 0U);                                                                              \
                for (; n; n--)                                                                                       \
                {                                                                                                    \
                    SW_CLOCK_CYCLE(clk, delay);                                                                      \
                }                                                                                                    \
            }                                                                                                        \
            PIN_SWDIO_OUT(dio, 1U);                                                                                  \
            return ((uint8_t)ack);                                                                                   \
        }                                                                                                            \
                                                                                                                     \
        if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT))                                               \
        {                                                                                                            \
            /* WAIT or FAULT response */                                                                             \
            if (SWD_DATA_PHASE && ((request & DAP_TRANSFER_RnW) != 0U))                                              \
            {                                                                                                        \
                for (n = 32U + 1U; n; n--)                                                                           \
                {                                                                                                    \
                    SW_CLOCK_CYCLE(clk, delay); /* Dummy Read RDATA[0:31] + Parity */                                \
                }                                                                                                    \
            }                                                                                                        \
            /* Turnaround */                                                                                         \
            for (n = SWD_TURNAROUND; n; n--)                                                                         \
            {                                                                                                        \
                SW_CLOCK_CYCLE(clk, delay);                                                                          \
            }                                                                                                        \
            PIN_SWDIO_OUT_ENABLE(dio);                                                                               \
            if (SWD_DATA_PHASE && ((request & DAP_TRANSFER_RnW) == 0U))                                              \
            {                                                                                                        \
                PIN_SWDIO_OUT(dio, 0U);                                                                              \
                for (n = 32U + 1U; n; n--)                                                                           \
                {                                                                                                    \
                    SW_CLOCK_CYCLE(clk, delay); /* Dummy Write WDATA[0:31] + Parity */                               \
                }                                                                                                    \
            }                                                                                                        \
            PIN_SWDIO_OUT(dio, 1U);                                                                                  \
            return ((uint8_t)ack);                                                                                   \
        }                                                                                                            \
                                                                                                                     \
        /* Protocol error */                                                                                         \
        for (n = SWD_TURNAROUND + 32U + 1U; n; n--)                                                                  \
        {                                                                                                            \
            SW_CLOCK_CYCLE(clk, delay); /* Back off data phase */                                                    \
        }                                                                                                            \
        PIN_SWDIO_OUT_ENABLE(dio);                                                                                   \
        PIN_SWDIO_OUT(dio, 1U);                                                                                      \
        return ((uint8_t)ack);                                                                                       \
    }
