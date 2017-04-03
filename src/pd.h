/*
 * PD Buddy - USB Power Delivery for everyone
 * Copyright (C) 2017 Clayton G. Hobbs <clay@lakeserv.net>
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

#ifndef PDB_PD_H
#define PDB_PD_H

#include <ch.h>


/*
 * Macros for working with USB Power Delivery messages.
 *
 * This file is mostly written from the PD Rev. 2.0 spec, but the header is
 * written from the Rev. 3.0 spec.
 */

/*
 * PD Header
 */
#define PD_HDR_MSGTYPE_SHIFT 0
#define PD_HDR_MSGTYPE (0x1F << PD_HDR_MSGTYPE_SHIFT)
#define PD_HDR_DATAROLE_SHIFT 5
#define PD_HDR_DATAROLE (0x1 << PD_HDR_DATAROLE_SHIFT)
#define PD_HDR_SPECREV_SHIFT 6
#define PD_HDR_SPECREV (0x3 << PD_HDR_SPECREV_SHIFT)
#define PD_HDR_POWERROLE_SHIFT 8
#define PD_HDR_POWERROLE (1 << PD_HDR_POWERROLE_SHIFT)
#define PD_HDR_MESSAGEID_SHIFT 9
#define PD_HDR_MESSAGEID (0x7 << PD_HDR_MESSAGEID_SHIFT)
#define PD_HDR_NUMOBJ_SHIFT 12
#define PD_HDR_NUMOBJ (0x7 << PD_HDR_NUMOBJ_SHIFT)
#define PD_HDR_EXT (1 << 15)

/* Message types */
#define PD_MSGTYPE_GET(msg) (((msg)->hdr & PD_HDR_MSGTYPE) >> PD_HDR_MSGTYPE_SHIFT)
/* Control Message */
#define PD_MSGTYPE_GOODCRC 0x01
#define PD_MSGTYPE_GOTOMIN 0x02
#define PD_MSGTYPE_ACCEPT 0x03
#define PD_MSGTYPE_REJECT 0x04
#define PD_MSGTYPE_PING 0x05
#define PD_MSGTYPE_PS_RDY 0x06
#define PD_MSGTYPE_GET_SOURCE_CAP 0x07
#define PD_MSGTYPE_GET_SINK_CAP 0x08
#define PD_MSGTYPE_DR_SWAP 0x09
#define PD_MSGTYPE_PR_SWAP 0x0A
#define PD_MSGTYPE_VCONN_SWAP 0x0B
#define PD_MSGTYPE_WAIT 0x0C
#define PD_MSGTYPE_SOFT_RESET 0x0D
#define PD_MSGTYPE_NOT_SUPPORTED 0x10
#define PD_MSGTYPE_GET_SOURCE_CAP_EXTENDED 0x11
#define PD_MSGTYPE_GET_STATUS 0x12
#define PD_MSGTYPE_FR_SWAP 0x13
#define PD_MSGTYPE_GET_PPS_STATUS 0x14
#define PD_MSGTYPE_GET_COUNTRY_CODES 0x15
/* Data Message */
#define PD_MSGTYPE_SOURCE_CAPABILITIES 0x01
#define PD_MSGTYPE_REQUEST 0x02
#define PD_MSGTYPE_BIST 0x03
#define PD_MSGTYPE_SINK_CAPABILITIES 0x04
#define PD_MSGTYPE_BATTERY_STATUS 0x05
#define PD_MSGTYPE_ALERT 0x06
#define PD_MSGTYPE_GET_COUNTRY_INFO 0x07
#define PD_MSGTYPE_VENDOR_DEFINED 0x0F
/* Extended Message */
#define PD_MSGTYPE_SOURCE_CAPABILITIES_EXTENDED 0x01
#define PD_MSGTYPE_STATUS 0x02
#define PD_MSGTYPE_GET_BATTERY_CAP 0x03
#define PD_MSGTYPE_GET_BATTERY_STATUS 0x04
#define PD_MSGTYPE_BATTERY_CAPABILITIES 0x05
#define PD_MSGTYPE_GET_MANUFACTURER_INFO 0x06
#define PD_MSGTYPE_MANUFACTURER_INFO 0x07
#define PD_MSGTYPE_SECURITY_REQUEST 0x08
#define PD_MSGTYPE_SECURITY_RESPONSE 0x09
#define PD_MSGTYPE_FIRMWARE_UPDATE_REQUEST 0x0A
#define PD_MSGTYPE_FIRMWARE_UPDATE_RESPONSE 0x0B
#define PD_MSGTYPE_PPS_STATUS 0x0C
#define PD_MSGTYPE_COUNTRY_INFO 0x0D
#define PD_MSGTYPE_COUNTRY_CODES 0x0E

/* Data roles */
#define PD_DATAROLE_UFP (0x0 << PD_HDR_DATAROLE_SHIFT)
#define PD_DATAROLE_DFP (0x1 << PD_HDR_DATAROLE_SHIFT)

/* Specification revisions */
#define PD_SPECREV_1_0 (0x0 << PD_HDR_SPECREV_SHIFT)
#define PD_SPECREV_2_0 (0x1 << PD_HDR_SPECREV_SHIFT)
#define PD_SPECREV_3_0 (0x2 << PD_HDR_SPECREV_SHIFT)

/* Port power roles */
#define PD_POWERROLE_SINK (0x0 << PD_HDR_POWERROLE_SHIFT)
#define PD_POWERROLE_SOURCE (0x1 << PD_HDR_POWERROLE_SHIFT)

/* Message ID */
#define PD_MESSAGEID_GET(msg) (((msg)->hdr & PD_HDR_MESSAGEID) >> PD_HDR_MESSAGEID_SHIFT)

/* Number of data objects */
#define PD_NUMOBJ(n) (((n) << PD_HDR_NUMOBJ_SHIFT) & PD_HDR_NUMOBJ)
#define PD_NUMOBJ_GET(msg) (((msg)->hdr & PD_HDR_NUMOBJ) >> PD_HDR_NUMOBJ_SHIFT)


/*
 * PD Power Data Object
 */
#define PD_PDO_TYPE_SHIFT 30
#define PD_PDO_TYPE (0x3 << PD_PDO_TYPE_SHIFT)

/* PDO types */
#define PD_PDO_TYPE_FIXED (0x0 << PD_PDO_TYPE_SHIFT)
#define PD_PDO_TYPE_BATTERY (0x1 << PD_PDO_TYPE_SHIFT)
#define PD_PDO_TYPE_VARIABLE (0x2 << PD_PDO_TYPE_SHIFT)

/* PD Source Fixed PDO */
#define PD_PDO_SRC_FIXED_DUAL_ROLE_PWR_SHIFT 29
#define PD_PDO_SRC_FIXED_DUAL_ROLE_PWR (1 << PD_PDO_SRC_FIXED_DUAL_ROLE_PWR_SHIFT)
#define PD_PDO_SRC_FIXED_USB_SUSPEND_SHIFT 28
#define PD_PDO_SRC_FIXED_USB_SUSPEND (1 << PD_PDO_SRC_FIXED_USB_SUSPEND_SHIFT)
#define PD_PDO_SRC_FIXED_UNCONSTRAINED_SHIFT 27
#define PD_PDO_SRC_FIXED_UNCONSTRAINED (1 << PD_PDO_SRC_FIXED_UNCONSTRAINED_SHIFT)
#define PD_PDO_SRC_FIXED_USB_COMMS_SHIFT 26
#define PD_PDO_SRC_FIXED_USB_COMMS (1 << PD_PDO_SRC_FIXED_USB_COMMS_SHIFT)
#define PD_PDO_SRC_FIXED_DUAL_ROLE_DATA_SHIFT 25
#define PD_PDO_SRC_FIXED_DUAL_ROLE_DATA (1 << PD_PDO_SRC_FIXED_DUAL_ROLE_DATA_SHIFT)
#define PD_PDO_SRC_FIXED_PEAK_CURRENT_SHIFT 20
#define PD_PDO_SRC_FIXED_PEAK_CURRENT (0x3 << PD_PDO_SRC_FIXED_PEAK_CURRENT_SHIFT)
#define PD_PDO_SRC_FIXED_VOLTAGE_SHIFT 10
#define PD_PDO_SRC_FIXED_VOLTAGE (0x3FF << PD_PDO_SRC_FIXED_VOLTAGE_SHIFT)
#define PD_PDO_SRC_FIXED_CURRENT_SHIFT 0
#define PD_PDO_SRC_FIXED_CURRENT (0x3FF << PD_PDO_SRC_FIXED_CURRENT_SHIFT)

/* PD Source Fixed PDO current */
#define PD_PDO_SRC_FIXED_CURRENT_GET(msg, i) (((msg)->obj[(i)] & PD_PDO_SRC_FIXED_CURRENT) >> PD_PDO_SRC_FIXED_CURRENT_SHIFT)

/* PD Source Fixed PDO voltage */
#define PD_PDO_SRC_FIXED_VOLTAGE_GET(msg, i) (((msg)->obj[(i)] & PD_PDO_SRC_FIXED_VOLTAGE) >> PD_PDO_SRC_FIXED_VOLTAGE_SHIFT)

/* TODO: other types of source PDO */

/* PD Sink Fixed PDO */
#define PD_PDO_SNK_FIXED_DUAL_ROLE_PWR_SHIFT 29
#define PD_PDO_SNK_FIXED_DUAL_ROLE_PWR (1 << PD_PDO_SNK_FIXED_DUAL_ROLE_PWR_SHIFT)
#define PD_PDO_SNK_FIXED_HIGHER_CAP_SHIFT 28
#define PD_PDO_SNK_FIXED_HIGHER_CAP (1 << PD_PDO_SNK_FIXED_HIGHER_CAP_SHIFT)
#define PD_PDO_SNK_FIXED_UNCONSTRAINED_SHIFT 27
#define PD_PDO_SNK_FIXED_UNCONSTRAINED (1 << PD_PDO_SNK_FIXED_UNCONSTRAINED_SHIFT)
#define PD_PDO_SNK_FIXED_USB_COMMS_SHIFT 26
#define PD_PDO_SNK_FIXED_USB_COMMS (1 << PD_PDO_SNK_FIXED_USB_COMMS_SHIFT)
#define PD_PDO_SNK_FIXED_DUAL_ROLE_DATA_SHIFT 25
#define PD_PDO_SNK_FIXED_DUAL_ROLE_DATA (1 << PD_PDO_SNK_FIXED_DUAL_ROLE_DATA_SHIFT)
#define PD_PDO_SNK_FIXED_VOLTAGE_SHIFT 10
#define PD_PDO_SNK_FIXED_VOLTAGE (0x3FF << PD_PDO_SNK_FIXED_VOLTAGE_SHIFT)
#define PD_PDO_SNK_FIXED_CURRENT_SHIFT 0
#define PD_PDO_SNK_FIXED_CURRENT (0x3FF << PD_PDO_SNK_FIXED_CURRENT_SHIFT)

/* PD Sink Fixed PDO current */
#define PD_PDO_SNK_FIXED_CURRENT_SET(i) (((i) << PD_PDO_SNK_FIXED_CURRENT_SHIFT) & PD_PDO_SNK_FIXED_CURRENT)

/* PD Sink Fixed PDO voltage */
#define PD_PDO_SNK_FIXED_VOLTAGE_SET(v) (((v) << PD_PDO_SNK_FIXED_VOLTAGE_SHIFT) & PD_PDO_SNK_FIXED_VOLTAGE)

/* TODO: other types of sink PDO */


/*
 * PD Request Data Object
 */
#define PD_RDO_OBJPOS_SHIFT 28
#define PD_RDO_OBJPOS (0x7 << PD_RDO_OBJPOS_SHIFT)
#define PD_RDO_GIVEBACK_SHIFT 27
#define PD_RDO_GIVEBACK (1 << PD_RDO_GIVEBACK_SHIFT)
#define PD_RDO_CAP_MISMATCH_SHIFT 26
#define PD_RDO_CAP_MISMATCH (1 << PD_RDO_CAP_MISMATCH_SHIFT)
#define PD_RDO_USB_COMMS_SHIFT 25
#define PD_RDO_USB_COMMS (1 << PD_RDO_USB_COMMS_SHIFT)
#define PD_RDO_NO_USB_SUSPEND_SHIFT 24
#define PD_RDO_NO_USB_SUSPEND (1 << PD_RDO_NO_USB_SUSPEND_SHIFT)

#define PD_RDO_OBJPOS_SET(i) (((i) << PD_RDO_OBJPOS_SHIFT) & PD_RDO_OBJPOS)

/* Fixed and Variable RDO, no giveback support */
#define PD_RDO_FV_CURRENT_SHIFT 10
#define PD_RDO_FV_CURRENT (0x3FF << PD_RDO_FV_CURRENT_SHIFT)
#define PD_RDO_FV_MAX_CURRENT_SHIFT 0
#define PD_RDO_FV_MAX_CURRENT (0x3FF << PD_RDO_FV_MAX_CURRENT_SHIFT)

#define PD_RDO_FV_CURRENT_SET(i) (((i) << PD_RDO_FV_CURRENT_SHIFT) & PD_RDO_FV_CURRENT)
#define PD_RDO_FV_MAX_CURRENT_SET(i) (((i) << PD_RDO_FV_MAX_CURRENT_SHIFT) & PD_RDO_FV_MAX_CURRENT)

/* TODO: other types of RDO */


/*
 * Time values
 *
 * Where a range is specified, the middle of the range (rounded down to the
 * nearest millisecond) is used.
 */
#define PD_T_HARD_RESET_COMPLETE MS2ST(4)
#define PD_T_PS_TRANSITION MS2ST(500)
#define PD_T_SENDER_RESPONSE MS2ST(27)
#define PD_T_TYPEC_SINK_WAIT_CAP MS2ST(465)


/*
 * Unit conversions
 *
 * V: volt
 * CV: centivolt
 * MV: millivolt
 * PDV: Power Delivery voltage unit (50 mV)
 * A: ampere
 * CA: centiampere
 * MA: milliampere
 * PDI: Power Delivery current unit (10 mA)
 */
#define PD_MV2PDV(mv) ((mv) / 50)
#define PD_PDV2MV(pdv) ((pdv) * 50)
#define PD_MA2PDI(ma) ((ma) / 10)
#define PD_PDI2MA(pdi) ((pdi) * 10)

/* Get portions of a PD voltage in more normal units */
#define PD_PDV_V(pdv) ((pdv) / 20)
#define PD_PDV_CV(pdv) (5 * ((pdv) % 20))

/* Get portions of a PD current in more normal units */
#define PD_PDI_A(pdv) ((pdv) / 100)
#define PD_PDI_CA(pdv) ((pdv) % 100)



#endif /* PDB_PD_H */
