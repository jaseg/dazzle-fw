#ifndef __DAZZLE_H__
#define __DAZZLE_H__

#include <ch.h>
#include <hal.h>


#define DAZZLE_PGOV_WA_SIZE 128
#define DAZZLE_PRIO_PRL_PGOV (NORMALPRIO-1)

#define DAZZLE_HCM_WA_SIZE 256
#define DAZZLE_PRIO_PRL_HCM (NORMALPRIO-1)
#define DAZZLE_HCM_MAX_REGISTERS 4
#define DAZZLE_HCM_MAX_BIT_DEPTH 10

struct power_governor_cfg {
    THD_WORKING_AREA(_wa, DAZZLE_PGOV_WA_SIZE);
    thread_t *thread;
};

struct high_current_modulation_cfg {
    struct {
        THD_WORKING_AREA(_wa, DAZZLE_HCM_WA_SIZE);
        thread_t *thread;
        uint8_t data_high[DAZZLE_HCM_MAX_BIT_DEPTH][DAZZLE_HCM_MAX_REGISTERS];
        uint8_t data_low[DAZZLE_HCM_MAX_BIT_DEPTH][DAZZLE_HCM_MAX_REGISTERS];
        bool is_blank;
        size_t bit_pos;
    } p;
    SPIDriver *spid_high;
    SPIDriver *spid_low;
    SPIConfig spic_high; /* Set only CR1 here (to set baudrate) */
    SPIConfig spic_low; /* Set only CR1 here (to set baudrate) */
    ioline_t sr_clear_line;
    ioline_t tlc_mode_line;
    int range_scale;
    float gamma;
    int front_porch;
    int high_strobe_allowance;
    size_t channel_count;
    size_t bit_depth;
    int high_offset_correction;
    int low_offset_correction;
    int base_divider;
    int prescaler;
    uint16_t val[DAZZLE_HCM_MAX_REGISTERS*8];
    int blank_period; /* automatically calculated if zero */
    int unblank_period_high[DAZZLE_HCM_MAX_BIT_DEPTH]; /* automatically calculated when offset_correction is given or when
                                                     element at index [0] is zero */
    int unblank_period_low[DAZZLE_HCM_MAX_BIT_DEPTH];
    uint8_t dot_correction[(DAZZLE_HCM_MAX_REGISTERS+1)/2*16];
    uint8_t high_channel_map[DAZZLE_HCM_MAX_REGISTERS*8];
    uint8_t low_channel_map[DAZZLE_HCM_MAX_REGISTERS*8];
};

struct dazzle_cfg {
    struct power_governor_cfg pgov;
    struct high_current_modulation_cfg hcm;
};

#endif /* __DAZZLE_H__ */
