
#include <ch.h>
#include <hal.h>

#include <dazzle.h>
#include <power_governor.h>

static THD_FUNCTION(PowerGovernor, vcfg) {
    struct dazzle_cfg *cfg = vcfg;

    while (true) {
        chThdSleepMilliseconds(2000);
        palSetLine(LINE_BIGBUCK_EN);
        chThdSleepMilliseconds(2000);
        palClearLine(LINE_BIGBUCK_EN);
    }
}

void dazzle_power_governor_run(struct dazzle_cfg *cfg) {
    cfg->pgov.thread = chThdCreateStatic(cfg->pgov._wa,
            sizeof(cfg->pgov._wa), DAZZLE_PRIO_PRL_PGOV, PowerGovernor, cfg);
}
