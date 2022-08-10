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

#include "device_policy_manager.h"
#include <stdint.h>
#include <hal.h>
#include <pd.h>
#include <unistd.h>
#include "config.h"

/* The current draw when the output is disabled [mA] */
#define DPM_MIN_CURRENT 100

bool pdbs_dpm_evaluate_capability(struct pdb_config *cfg, const union pd_msg *caps, union pd_msg *request)
{
    if (caps != NULL) {
        /* Look at the PDOs to see if one matches our desires */
        size_t numobj = PD_NUMOBJ_GET(caps);
        ssize_t max_index = -1;
        int min_voltage = 20000;
        int max_current = 0;
        int max_power = 0;
        
        for (size_t i=0; i<numobj; i++) {
            /* If we have a fixed PDO, its V equals our desired V, and its I is
             * at least our desired I */
            if ((caps->obj[i] & PD_PDO_TYPE) == PD_PDO_TYPE_FIXED) {
                int this_voltage = PD_PDV2MV(PD_PDO_SRC_FIXED_VOLTAGE_GET(caps->obj[i]));
                int this_current = PD_PDI2MA(PD_PDO_SRC_FIXED_CURRENT_GET(caps->obj[i]));
                int this_power = (this_voltage * this_current + 500) / 1000; /* mW */

                /* Choose the entry providing the most power. When we find multiple entries providing the same power, choose
                 * the one with the lowest voltage to reduce buck converter losses. */
                if ((this_power < max_power) || ((this_power == max_power) && (this_voltage > min_voltage))) {
                    continue;
                }

                max_power = this_power;
                min_voltage = this_voltage;
                max_current = this_current;
                max_index = i;
            }
        }

        if (max_index >= 0) {
            /* We got what we wanted, so build a request for that */
            request->hdr = cfg->pe.hdr_template | PD_MSGTYPE_REQUEST | PD_NUMOBJ(1);
            /* GiveBack disabled */
            request->obj[0] = PD_RDO_FV_MAX_CURRENT_SET(PD_MA2PDI(max_current))
                              | PD_RDO_FV_CURRENT_SET(PD_MA2PDI(max_current))
                              | PD_RDO_NO_USB_SUSPEND | PD_RDO_OBJPOS_SET(max_index + 1);
            return true;
        }

        request->obj[0] |= PD_RDO_CAP_MISMATCH;
        /* Nothing matched (or no configuration), so get 5 V at low current */
        request->hdr = cfg->pe.hdr_template | PD_MSGTYPE_REQUEST | PD_NUMOBJ(1);
        request->obj[0] = PD_RDO_FV_MAX_CURRENT_SET(PD_MA2PDI(DPM_MIN_CURRENT))
                          | PD_RDO_FV_CURRENT_SET(PD_MA2PDI(DPM_MIN_CURRENT))
                          | PD_RDO_NO_USB_SUSPEND
                          | PD_RDO_OBJPOS_SET(1);
        return false;
    }
    return true;
}

void pdbs_dpm_get_sink_capability(struct pdb_config *cfg, union pd_msg *cap)
{
    /* Keep track of how many PDOs we've added */
    int numobj = 0;
    /* Get the current configuration */

    cap->obj[numobj++] = PD_PDO_TYPE_FIXED
                             | PD_PDO_SNK_FIXED_VOLTAGE_SET(PD_MV2PDV(20000))
                             | PD_PDO_SNK_FIXED_CURRENT_SET(3000);
    cap->obj[0] |= PD_PDO_SNK_FIXED_UNCONSTRAINED;

    cap->obj[numobj++] = PD_PDO_TYPE_FIXED
                             | PD_PDO_SNK_FIXED_VOLTAGE_SET(PD_MV2PDV(15000))
                             | PD_PDO_SNK_FIXED_CURRENT_SET(3000);

    cap->obj[numobj++] = PD_PDO_TYPE_FIXED
                             | PD_PDO_SNK_FIXED_VOLTAGE_SET(PD_MV2PDV(9000))
                             | PD_PDO_SNK_FIXED_CURRENT_SET(3000);

    cap->obj[numobj++] = PD_PDO_TYPE_FIXED
                             | PD_PDO_SNK_FIXED_VOLTAGE_SET(PD_MV2PDV(5000))
                             | PD_PDO_SNK_FIXED_CURRENT_SET(3000);

    /* Set the Sink_Capabilities message header */
    cap->hdr = cfg->pe.hdr_template | PD_MSGTYPE_SINK_CAPABILITIES | PD_NUMOBJ(numobj);
}

bool pdbs_dpm_giveback_enabled(struct pdb_config *cfg)
{
    (void) cfg;
    return false;
}

bool pdbs_dpm_evaluate_typec_current(struct pdb_config *cfg, enum fusb_typec_current tcc) {
    return false;
}

void pdbs_dpm_pd_start(struct pdb_config *cfg)
{
    (void) cfg;
}

void pdbs_dpm_transition_default(struct pdb_config *cfg)
{
}

void pdbs_dpm_transition_min(struct pdb_config *cfg)
{
}

void pdbs_dpm_transition_standby(struct pdb_config *cfg)
{
}

void pdbs_dpm_transition_requested(struct pdb_config *cfg)
{
}

void pdbs_dpm_transition_typec(struct pdb_config *cfg)
{
}
