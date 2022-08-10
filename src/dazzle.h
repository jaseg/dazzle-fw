#ifndef __DAZZLE_H__
#define __DAZZLE_H__

#include <ch.h>
#include <hal.h>


#define DAZZLE_PGOV_WA_SIZE 128
#define DAZZLE_PRIO_PRL_PGOV (NORMALPRIO-1)

#define DAZZLE_HCM_WA_SIZE 128
#define DAZZLE_PRIO_PRL_HCM (NORMALPRIO-1)
#define DAZZLE_HCM_MAX_REGISTERS 4

struct power_governor_cfg {
    THD_WORKING_AREA(_wa, DAZZLE_PGOV_WA_SIZE);
    thread_t *thread;
};

struct high_current_modulation_cfg {
    THD_WORKING_AREA(_wa, DAZZLE_HCM_WA_SIZE);
    thread_t *thread;
    SPIDriver *spid;
    SPIConfig spic;
    size_t num_registers;
    uint8_t sr_data[DAZZLE_HCM_MAX_REGISTERS];
};

struct dazzle_cfg {
    struct power_governor_cfg pgov;
    struct high_current_modulation_cfg hcm;
};

#endif /* __DAZZLE_H__ */
