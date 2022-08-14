/*
 * PD Buddy Sink Firmware - Smart power jack for USB Power Delivery
 * Copyright (C) 2017-2018 Clayton G. Hobbs <clay@lakeserv.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
   ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include <ch.h>
#include <hal.h>
#include <chcore.h>

#include <pdb.h>
#include <pd.h>
#include "device_policy_manager.h"
#include "stm32f072_bootloader.h"
#include "chprintf.h"
#include <string.h>
#include <stdio.h>
#include <dazzle.h>
#include <power_governor.h>
#include <high_current_modulation.h>

/*
 * I2C configuration object.
 * I2C2_TIMINGR: 1000 kHz with I2CCLK = 48 MHz, rise time = 100 ns,
 *               fall time = 10 ns (0x00700818)
 */
static const I2CConfig i2c2config = {
    0x00700818,
    0,
    0
};

static struct dazzle_cfg dazzle_cfg = {
    .hcm = {
        .spid = &SPID2,
        .spic = { .cr1 = SPI_CR1_BR_1 | SPI_CR1_BR_0 },
        .pwmd = &PWMD15,
        .pwmc = {
            .frequency = 8000000,
            .channels = { {.mode = PWM_COMPLEMENTARY_OUTPUT_ACTIVE_HIGH }}
        },
        .clear_line = LINE_SR_CLR,
        .channel_count = 32,
        .bit_depth = 10,
        .offset_correction = 0,
    },
};

/*
 * PD Buddy firmware library configuration object
 */
static struct pdb_config pdb_config = {
    .fusb = {
        &I2CD2,
        FUSB302B_ADDR,
        LINE_INT_N
    },
    .dpm = {
        pdbs_dpm_evaluate_capability,
        pdbs_dpm_get_sink_capability,
        pdbs_dpm_giveback_enabled,
        pdbs_dpm_evaluate_typec_current,
        pdbs_dpm_pd_start,
        pdbs_dpm_transition_default,
        pdbs_dpm_transition_min,
        pdbs_dpm_transition_standby,
        pdbs_dpm_transition_requested,
        pdbs_dpm_transition_typec,
        NULL /* not_supported_received */
    },
    .dpm_data = NULL,
};

struct __attribute__((packed)) ContextStateFrame {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t return_address;
  uint32_t xpsr;
};

__attribute__((__naked__)) void HardFault_Handler(void) {
    PORT_IRQ_PROLOGUE();
    struct ContextStackFrame *frame;
    if (_saved_lr & 4) { /* _saved_lr from PORT_IRQ_PROLOGUE */
        frame = (struct ContextStackFrame *)__get_PSP();
    } else {
        frame = (struct ContextStackFrame *)__get_MSP();
    }
    (void) frame;
    __BKPT(42);
    PORT_IRQ_EPILOGUE();
}

int main(void) {
    halInit();
    chSysInit();
    // FIXME DEBUG i2cStart(pdb_config.fusb.i2cp, &i2c2config);
    chThdSleepMilliseconds(100);

    /* Start the USB Power Delivery threads */
    // FIXME DEBUG pdb_init(&pdb_config);
    chThdSleepMilliseconds(100);
    // FIXME DEBUG chEvtSignal(pdb_config.pe.thread, PDB_EVT_PE_NEW_POWER);
    dazzle_power_governor_run(&dazzle_cfg);
    dazzle_high_current_modulation_run(&dazzle_cfg.hcm);
    /* Wait, letting all the other threads do their work. */
    while (true) {
        chThdSleepMilliseconds(1000);
    }
}
