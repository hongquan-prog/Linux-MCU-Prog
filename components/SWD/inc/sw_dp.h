#pragma once

#include "compiler.h"

#define SWD_DATA_PHASE 0
#define SWD_TURNAROUND 1
#define SWD_IDLE_CYCLES 0

/// Default communication speed on the Debug Access Port for SWD and JTAG mode.
/// Used to initialize the default SWD/JTAG clock frequency.
/// The command \ref DAP_SWJ_Clock can be used to overwrite this default setting.
#define DAP_DEFAULT_SWJ_CLOCK 5000000U ///< Default SWD/JTAG clock frequency in Hz.

// Configurable delay for clock generation
#define DELAY_SLOW_CYCLES 4U // Number of cycles for one iteration
// Fixed delay for fast clock generation
#define DELAY_FAST_CYCLES 3U // Number of cycles: 0..3

#define SWD_PIN_OPERATIONS_DEFINE(INIT, SET, CLR, READ, WRITE, CLK_BASE, CLK_PIN, DIO_BASE, DIO_PIN) \
                                                                                                     \
    __STATIC_INLINE void PORT_INIT()                                                                 \
    {                                                                                                \
        INIT(DIO_BASE, DIO_PIN, true);                                                               \
        INIT(CLK_BASE, CLK_PIN, true);                                                               \
        SET(DIO_BASE, DIO_PIN);                                                                      \
        SET(CLK_BASE, CLK_PIN);                                                                      \
        CLR(DIO_BASE, DIO_PIN);                                                                      \
        SET(CLK_BASE, CLK_PIN);                                                                      \
        LOG_INFO("SWD GPIO init done");                                                              \
    }                                                                                                \
                                                                                                     \
    __STATIC_INLINE void PORT_OFF(void)                                                              \
    {                                                                                                \
        INIT(DIO_BASE, DIO_PIN, false);                                                              \
        INIT(CLK_BASE, CLK_PIN, false);                                                              \
        LOG_INFO("SWD GPIO Reset done");                                                             \
    }                                                                                                \
                                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWCLK_TCK_SET(void)                                                \
    {                                                                                                \
        SET(CLK_BASE, CLK_PIN);                                                                      \
    }                                                                                                \
                                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWCLK_TCK_CLR(void)                                                \
    {                                                                                                \
        CLR(CLK_BASE, CLK_PIN);                                                                      \
    }                                                                                                \
                                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_TMS_SET(void)                                                \
    {                                                                                                \
        SET(DIO_BASE, DIO_PIN);                                                                      \
    }                                                                                                \
                                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_TMS_CLR(void)                                                \
    {                                                                                                \
        CLR(DIO_BASE, DIO_PIN);                                                                      \
    }                                                                                                \
                                                                                                     \
    __STATIC_FORCEINLINE uint32_t PIN_SWDIO_IN(void)                                                 \
    {                                                                                                \
        return READ(DIO_BASE, DIO_PIN);                                                              \
    }                                                                                                \
                                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_OUT(uint32_t bit)                                            \
    {                                                                                                \
        WRITE(DIO_BASE, DIO_PIN, bit & 0x1);                                                         \
    }                                                                                                \
                                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_OUT_ENABLE(void)                                             \
    {                                                                                                \
        INIT(DIO_BASE, DIO_PIN, true);                                                               \
    }                                                                                                \
                                                                                                     \
    __STATIC_FORCEINLINE void PIN_SWDIO_OUT_DISABLE(void)                                            \
    {                                                                                                \
        INIT(DIO_BASE, DIO_PIN, false);                                                              \
    }                                                                                                \
                                                                                                     \
    __STATIC_FORCEINLINE void PIN_nRESET_OUT(uint32_t bit)                                           \
    {                                                                                                \
    }

// SW Macros
#define PIN_SWCLK_SET PIN_SWCLK_TCK_SET
#define PIN_SWCLK_CLR PIN_SWCLK_TCK_CLR

#define SW_CLOCK_CYCLE() \
    PIN_SWCLK_CLR();     \
    PIN_DELAY();         \
    PIN_SWCLK_SET();     \
    PIN_DELAY()

#define SW_WRITE_BIT(bit) \
    PIN_SWDIO_OUT(bit);   \
    PIN_SWCLK_CLR();      \
    PIN_DELAY();          \
    PIN_SWCLK_SET();      \
    PIN_DELAY()

#define SW_READ_BIT(bit)  \
    PIN_SWCLK_CLR();      \
    PIN_DELAY();          \
    bit = PIN_SWDIO_IN(); \
    PIN_SWCLK_SET();      \
    PIN_DELAY()

__STATIC_FORCEINLINE void PIN_DELAY_SLOW(volatile uint32_t delay)
{
    while (delay)
    {
        --delay;
    }
}

__STATIC_FORCEINLINE void PIN_DELAY_FAST(void)
{
}

#define SWJ_SEQUENCE_DEFINE(name)                         \
    static void name(uint32_t count, const uint8_t *data) \
    {                                                     \
        uint32_t val;                                     \
        uint32_t n;                                       \
                                                          \
        val = 0U;                                         \
        n = 0U;                                           \
        while (count--)                                   \
        {                                                 \
            if (n == 0U)                                  \
            {                                             \
                val = *data++;                            \
                n = 8U;                                   \
            }                                             \
            if (val & 1U)                                 \
            {                                             \
                PIN_SWDIO_TMS_SET();                      \
            }                                             \
            else                                          \
            {                                             \
                PIN_SWDIO_TMS_CLR();                      \
            }                                             \
            SW_CLOCK_CYCLE();                             \
            val >>= 1;                                    \
            n--;                                          \
        }                                                 \
    }

#define SWD_TRANSFER_DEFINE(name)                                            \
    uint8_t name(uint32_t request, uint32_t *data)                           \
    {                                                                        \
        uint32_t ack;                                                        \
        uint32_t bit;                                                        \
        uint32_t val;                                                        \
        uint32_t parity;                                                     \
                                                                             \
        uint32_t n;                                                          \
                                                                             \
        /* Packet Request */                                                 \
        parity = 0U;                                                         \
        SW_WRITE_BIT(1U); /* Start Bit */                                    \
        bit = request >> 0;                                                  \
        SW_WRITE_BIT(bit); /* APnDP Bit */                                   \
        parity += bit;                                                       \
        bit = request >> 1;                                                  \
        SW_WRITE_BIT(bit); /* RnW Bit */                                     \
        parity += bit;                                                       \
        bit = request >> 2;                                                  \
        SW_WRITE_BIT(bit); /* A2 Bit */                                      \
        parity += bit;                                                       \
        bit = request >> 3;                                                  \
        SW_WRITE_BIT(bit); /* A3 Bit */                                      \
        parity += bit;                                                       \
        SW_WRITE_BIT(parity); /* Parity Bit */                               \
        SW_WRITE_BIT(0U);     /* Stop Bit */                                 \
        SW_WRITE_BIT(1U);     /* Park Bit */                                 \
                                                                             \
        /* Turnaround */                                                     \
        PIN_SWDIO_OUT_DISABLE();                                             \
        for (n = SWD_TURNAROUND; n; n--)                                     \
        {                                                                    \
            SW_CLOCK_CYCLE();                                                \
        }                                                                    \
                                                                             \
        /* Acknowledge response */                                           \
        SW_READ_BIT(bit);                                                    \
        ack = bit << 0;                                                      \
        SW_READ_BIT(bit);                                                    \
        ack |= bit << 1;                                                     \
        SW_READ_BIT(bit);                                                    \
        ack |= bit << 2;                                                     \
        /* OK response */                                                    \
        if (ack == DAP_TRANSFER_OK)                                          \
        {                                                                    \
            /* Data transfer */                                              \
            if (request & DAP_TRANSFER_RnW)                                  \
            {                                                                \
                /* Read data */                                              \
                val = 0U;                                                    \
                parity = 0U;                                                 \
                for (n = 32U; n; n--)                                        \
                {                                                            \
                    SW_READ_BIT(bit); /* Read RDATA[0:31] */                 \
                    parity += bit;                                           \
                    val >>= 1;                                               \
                    val |= bit << 31;                                        \
                }                                                            \
                SW_READ_BIT(bit); /* Read Parity */                          \
                if ((parity ^ bit) & 1U)                                     \
                {                                                            \
                    ack = DAP_TRANSFER_ERROR;                                \
                }                                                            \
                if (data)                                                    \
                {                                                            \
                    *data = val;                                             \
                }                                                            \
                /* Turnaround */                                             \
                for (n = SWD_TURNAROUND; n; n--)                             \
                {                                                            \
                    SW_CLOCK_CYCLE();                                        \
                }                                                            \
                PIN_SWDIO_OUT_ENABLE();                                      \
            }                                                                \
            else                                                             \
            {                                                                \
                /* Turnaround */                                             \
                for (n = SWD_TURNAROUND; n; n--)                             \
                {                                                            \
                    SW_CLOCK_CYCLE();                                        \
                }                                                            \
                PIN_SWDIO_OUT_ENABLE();                                      \
                /* Write data */                                             \
                val = *data;                                                 \
                parity = 0U;                                                 \
                for (n = 32U; n; n--)                                        \
                {                                                            \
                    SW_WRITE_BIT(val); /* Write WDATA[0:31] */               \
                    parity += val;                                           \
                    val >>= 1;                                               \
                }                                                            \
                SW_WRITE_BIT(parity); /* Write Parity Bit */                 \
            }                                                                \
            /* Idle cycles */                                                \
            n = SWD_IDLE_CYCLES;                                             \
            if (n)                                                           \
            {                                                                \
                PIN_SWDIO_OUT(0U);                                           \
                for (; n; n--)                                               \
                {                                                            \
                    SW_CLOCK_CYCLE();                                        \
                }                                                            \
            }                                                                \
            PIN_SWDIO_OUT(1U);                                               \
            return ((uint8_t)ack);                                           \
        }                                                                    \
                                                                             \
        if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT))       \
        {                                                                    \
            /* WAIT or FAULT response */                                     \
            if (SWD_DATA_PHASE && ((request & DAP_TRANSFER_RnW) != 0U))      \
            {                                                                \
                for (n = 32U + 1U; n; n--)                                   \
                {                                                            \
                    SW_CLOCK_CYCLE(); /* Dummy Read RDATA[0:31] + Parity */  \
                }                                                            \
            }                                                                \
            /* Turnaround */                                                 \
            for (n = SWD_TURNAROUND; n; n--)                                 \
            {                                                                \
                SW_CLOCK_CYCLE();                                            \
            }                                                                \
            PIN_SWDIO_OUT_ENABLE();                                          \
            if (SWD_DATA_PHASE && ((request & DAP_TRANSFER_RnW) == 0U))      \
            {                                                                \
                PIN_SWDIO_OUT(0U);                                           \
                for (n = 32U + 1U; n; n--)                                   \
                {                                                            \
                    SW_CLOCK_CYCLE(); /* Dummy Write WDATA[0:31] + Parity */ \
                }                                                            \
            }                                                                \
            PIN_SWDIO_OUT(1U);                                               \
            return ((uint8_t)ack);                                           \
        }                                                                    \
                                                                             \
        /* Protocol error */                                                 \
        for (n = SWD_TURNAROUND + 32U + 1U; n; n--)                          \
        {                                                                    \
            SW_CLOCK_CYCLE(); /* Back off data phase */                      \
        }                                                                    \
        PIN_SWDIO_OUT_ENABLE();                                              \
        PIN_SWDIO_OUT(1U);                                                   \
        return ((uint8_t)ack);                                               \
    }
