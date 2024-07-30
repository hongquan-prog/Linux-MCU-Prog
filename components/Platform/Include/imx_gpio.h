/*
 * SPDX-FileCopyrightText: 2020-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "fsl_rgpio.h"
#include "compiler.h"

/* swd */
#define GPIO_PIN_SWCLK      4
#define GPIO_PIN_SWDIO      5

#define GPIO_PORT_SWCLK      GPIO2
#define GPIO_PORT_SWDIO      GPIO2

void imx_init_swd_pins(void);
void imx_reset_dap_pins(void);
