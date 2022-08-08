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

#include "config.h"


/* The current draw when the output is disabled */
#define DPM_MIN_CURRENT PD_MA2PDI(30)

/*
 * Return the current specified by the given PDBS configuration object at the
 * given voltage (in millivolts), in centiamperes.
 */

bool pdbs_dpm_evaluate_capability(struct pdb_config *cfg,
                                  const union pd_msg *caps, union pd_msg *request)
{

    /* Cast the dpm_data to the right type */
    struct pdbs_dpm_data *dpm_data = cfg->dpm_data;

    /* Update the stored Source_Capabilities */
    if (caps != NULL) {
        if (dpm_data->capabilities != NULL) {
            chPoolFree(&pdb_msg_pool, (union pd_msg *) dpm_data->capabilities);
        }
        dpm_data->capabilities = caps;
    } else {
        /* No new capabilities; use a shorter name for the stored ones. */
        caps = dpm_data->capabilities;
    }

    /* Get the number of PDOs */
    uint8_t numobj = PD_NUMOBJ_GET(caps);

    /* Get whether or not the power supply is constrained */
    dpm_data->_unconstrained_power = caps->obj[0] & PD_PDO_SRC_FIXED_UNCONSTRAINED;

    uint16_t current = 0;
    uint16_t voltage = 20000;

    /* Make sure we have configuration */
    if (dpm_data->output_enabled) {
        /* Look at the PDOs to see if one matches our desires */
        for (uint8_t i = 0; i < numobj; i++) {
            /* If we have a fixed PDO, its V equals our desired V, and its I is
             * at least our desired I */
            if ((caps->obj[i] & PD_PDO_TYPE) == PD_PDO_TYPE_FIXED
                    && PD_PDO_SRC_FIXED_VOLTAGE_GET(caps->obj[i]) == PD_MV2PDV(voltage)
                    && PD_PDO_SRC_FIXED_CURRENT_GET(caps->obj[i]) >= current) {
                /* We got what we wanted, so build a request for that */
                request->hdr = cfg->pe.hdr_template | PD_MSGTYPE_REQUEST | PD_NUMOBJ(1);
                /* GiveBack disabled */
                request->obj[0] = PD_RDO_FV_MAX_CURRENT_SET(current)
                                  | PD_RDO_FV_CURRENT_SET(current)
                                  | PD_RDO_NO_USB_SUSPEND | PD_RDO_OBJPOS_SET(i + 1);
                if (dpm_data->usb_comms) {
                    request->obj[0] |= PD_RDO_USB_COMMS;
                }

                /* Update requested voltage */
                dpm_data->_requested_voltage = PD_PDV2MV(PD_MV2PDV(voltage));

                dpm_data->_capability_match = true;
                return true;
            }
        }
    }

    /* Nothing matched (or no configuration), so get 5 V at low current */
    request->hdr = cfg->pe.hdr_template | PD_MSGTYPE_REQUEST | PD_NUMOBJ(1);
    request->obj[0] = PD_RDO_FV_MAX_CURRENT_SET(DPM_MIN_CURRENT)
                      | PD_RDO_FV_CURRENT_SET(DPM_MIN_CURRENT)
                      | PD_RDO_NO_USB_SUSPEND
                      | PD_RDO_OBJPOS_SET(1);
    /* If the output is enabled and we got here, it must be a capability
     * mismatch. */
    if (dpm_data->output_enabled) {
        request->obj[0] |= PD_RDO_CAP_MISMATCH;
    }
    /* If we can do USB communications, tell the power supply */
    if (dpm_data->usb_comms) {
        request->obj[0] |= PD_RDO_USB_COMMS;
    }

    /* Update requested voltage */
    dpm_data->_requested_voltage = 5000;

    /* At this point, we have a capability match iff the output is disabled */
    dpm_data->_capability_match = !dpm_data->output_enabled;
    return !dpm_data->output_enabled;
}

void pdbs_dpm_get_sink_capability(struct pdb_config *cfg, union pd_msg *cap)
{
    /* Keep track of how many PDOs we've added */
    int numobj = 0;
    /* Get the current configuration */
    /* Cast the dpm_data to the right type */

    /* If we have no configuration or want something other than 5 V, add a PDO
     * for vSafe5V */
    cap->obj[numobj++] = PD_PDO_TYPE_FIXED
                             | PD_PDO_SNK_FIXED_VOLTAGE_SET(PD_MV2PDV(20000))
                             | PD_PDO_SNK_FIXED_CURRENT_SET(3000);
    cap->obj[0] |= PD_PDO_SNK_FIXED_UNCONSTRAINED;
    cap->obj[0] |= PD_PDO_SNK_FIXED_USB_COMMS;

    /* Set the Sink_Capabilities message header */
    cap->hdr = cfg->pe.hdr_template | PD_MSGTYPE_SINK_CAPABILITIES | PD_NUMOBJ(numobj);
}

bool pdbs_dpm_giveback_enabled(struct pdb_config *cfg)
{
    (void) cfg;
    return false;
}

bool pdbs_dpm_evaluate_typec_current(struct pdb_config *cfg,
                                     enum fusb_typec_current tcc)
{
    struct pdbs_dpm_data *dpm_data = cfg->dpm_data;
    dpm_data->_capability_match = false;
    return false;
}

void pdbs_dpm_pd_start(struct pdb_config *cfg)
{
    (void) cfg;
}

/*
 * Set the output state, with LED indication.
 */
static void dpm_output_set(struct pdbs_dpm_data *dpm_data, bool state)
{
    /* Update the present voltage */
    dpm_data->_present_voltage = dpm_data->_requested_voltage;

    /* Set the power output */
    if (state && dpm_data->output_enabled) {
        /* Turn the output on */
    } else {
        /* Turn the output off */
    }
}

void pdbs_dpm_transition_default(struct pdb_config *cfg)
{
    /* Cast the dpm_data to the right type */
    struct pdbs_dpm_data *dpm_data = cfg->dpm_data;

    /* Pretend we requested 5 V */
    dpm_data->_requested_voltage = 5000;
    /* Turn the output off */
    dpm_output_set(cfg->dpm_data, false);
}

void pdbs_dpm_transition_min(struct pdb_config *cfg)
{
    dpm_output_set(cfg->dpm_data, false);
}

void pdbs_dpm_transition_standby(struct pdb_config *cfg)
{
    /* Cast the dpm_data to the right type */
    struct pdbs_dpm_data *dpm_data = cfg->dpm_data;

    /* If the voltage is changing, enter Sink Standby */
    if (dpm_data->_requested_voltage != dpm_data->_present_voltage) {
        /* For the PD Buddy Sink, entering Sink Standby is equivalent to
         * turning the output off.  However, we don't want to change the LED
         * state for standby mode. */
    }
}

void pdbs_dpm_transition_requested(struct pdb_config *cfg)
{
    /* Cast the dpm_data to the right type */
    struct pdbs_dpm_data *dpm_data = cfg->dpm_data;

    dpm_output_set(cfg->dpm_data, dpm_data->_capability_match);
}

void pdbs_dpm_transition_typec(struct pdb_config *cfg)
{
    /* Cast the dpm_data to the right type */
    struct pdbs_dpm_data *dpm_data = cfg->dpm_data;

    /* Set the output, only setting the LED status if it wasn't set above */
    dpm_output_set(cfg->dpm_data, dpm_data->_capability_match);
}
