#ifndef __DAZZLE_H__
#define __DAZZLE_H__

#include <ch.h>
#include <hal.h>


#define DAZZLE_PGOV_WA_SIZE 128
#define DAZZLE_PRIO_PRL_PGOV (NORMALPRIO-1)

#define DAZZLE_HCM_WA_SIZE 128
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
        uint8_t sr_data[DAZZLE_HCM_MAX_BIT_DEPTH][DAZZLE_HCM_MAX_REGISTERS];
        bool is_blank;
        size_t bit_pos;
    } p;
    SPIDriver *spid;
    SPIConfig spic; /* Set only CR1 here (to set baudrate) */
    ioline_t clear_line;
    size_t channel_count;
    size_t bit_depth;
    int offset_correction;
    uint16_t val[DAZZLE_HCM_MAX_REGISTERS*8];
    int blank_period; /* automatically calculated if zero */
    int unblank_period[DAZZLE_HCM_MAX_BIT_DEPTH]; /* automatically calculated when offset_correction is given or when
                                                     element at index [0] is zero */
};

struct dazzle_cfg {
    struct power_governor_cfg pgov;
    struct high_current_modulation_cfg hcm;
};

#endif /* __DAZZLE_H__ */
