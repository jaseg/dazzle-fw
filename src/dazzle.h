#ifndef __DAZZLE_H__
#define __DAZZLE_H__

#include <ch.h>


#define DAZZLE_PGOV_WA_SIZE 128
#define DAZZLE_PRIO_PRL_PGOV (NORMALPRIO-1)

struct power_governor_cfg {
    THD_WORKING_AREA(_wa, DAZZLE_PGOV_WA_SIZE);
    thread_t *thread;
};

struct dazzle_cfg {
    struct power_governor_cfg pgov;
};

#endif /* __DAZZLE_H__ */
