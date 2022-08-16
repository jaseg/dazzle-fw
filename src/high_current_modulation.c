
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <ch.h>
#include <hal.h>

#include <dazzle.h>
#include <power_governor.h>

OSAL_IRQ_HANDLER(STM32_TIM15_UP_HANDLER) {
  OSAL_IRQ_PROLOGUE();
  OSAL_IRQ_EPILOGUE();
}


static uint32_t xorshift32(void) {
    static uint32_t st = 1;
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	uint32_t x = st;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return st = x;
}


static void precalc_modulation(struct high_current_modulation_cfg *cfg) {
    /* transpose brightness values from cfg->val to cfg->data_{high,low} */
    for (size_t j=0; j<cfg->bit_depth; j++) {
        for (size_t i=0; i<cfg->channel_count/8; i++) {
            cfg->p.data_high[j][i] = 0;
        }
        for (size_t i=0; i<cfg->channel_count/8; i++) {
            for (size_t k=0; k<8; k++) {
                if (cfg->val[i*8+k] & (1<<j))
                    cfg->p.data_high[j][i] |= (1<<k);
            }
        }
    }
}

static THD_FUNCTION(HighCurrentModulation, vcfg) {
    struct high_current_modulation_cfg *cfg = vcfg;

    int j = 0;
    while (true) {
        chThdSleepMilliseconds(7);
        float input = j;
        if (input > (1<<cfg->bit_depth)-1) {
            input = (1<<cfg->bit_depth)-1;
        }
        j = (j+3) % ((1<<cfg->bit_depth) + 200);
        float gamma = 2.8;
        int max = (1<<cfg->bit_depth) - 1;
        float value = powf(input / max, gamma) * max + 0.5;
        for (size_t i=0; i<32; i++) {
            cfg->val[i] = value;
        }
        precalc_modulation(cfg);
    }
}

static struct high_current_modulation_cfg *tim15_cfg;
OSAL_IRQ_HANDLER(STM32_TIM15_HANDLER) {
    OSAL_IRQ_PROLOGUE();
    TIM15->SR = 0;

    static size_t bit_pos_lookup[10] = {0,1,2,3,4,5,6,7,8,9};
    struct high_current_modulation_cfg *cfg = tim15_cfg;
    bool is_blank = cfg->p.is_blank;
    cfg->p.is_blank = !is_blank;
    if (is_blank) {
        palSetLine(cfg->sr_clear_line);

        /*
        size_t bit_pos = xorshift32() % cfg->bit_depth + 4;
        size_t duration_factor = 1;
        if (bit_pos >= cfg->bit_depth-2 && bit_pos <= cfg->bit_depth-1) {
            bit_pos = cfg->bit_depth-2;
            duration_factor = 2;
        } else if (bit_pos >= cfg->bit_depth) {
            bit_pos = cfg->bit_depth-1;
            duration_factor = 4;
        }
        */
        size_t bit_pos = (cfg->p.bit_pos + 1);
        if (bit_pos == cfg->bit_depth) {
            bit_pos = 0;
            /* dither timer frequency to spread out coil whine */
            //TIM15->PSC = 47 - 10 + xorshift32()%21; FIXME
        }
        cfg->p.bit_pos = bit_pos;
        bit_pos = bit_pos_lookup[bit_pos];

        spiStartSendI(cfg->spid_high, cfg->channel_count/8, cfg->p.data_high[bit_pos]);
        TIM15->ARR = cfg->unblank_period_high[bit_pos];
        TIM1->CCR3 = cfg->unblank_period_high[bit_pos] - 24; /* FIXME debug code */

    } else {
        palClearLine(cfg->sr_clear_line);
        TIM15->ARR = cfg->blank_period;
        TIM1->CCR3 = 0; /* FIXME debug code */
    }

    OSAL_IRQ_EPILOGUE();
}

void dazzle_high_current_modulation_run(struct high_current_modulation_cfg *cfg) {
    cfg->p.is_blank = false;
    cfg->p.bit_pos = 0;

    cfg->spic_high.circular = false;
    cfg->spic_high.slave = false;
    cfg->spic_high.data_cb = NULL;
    cfg->spic_high.error_cb = NULL;
    cfg->spic_high.cr2 = 0;
    cfg->spic_high.cr1 = SPI_CR1_BR_1 | SPI_CR1_BR_0;
    spiStart(cfg->spid_high, &cfg->spic_high);

    cfg->spic_low.circular = false;
    cfg->spic_low.slave = false;
    cfg->spic_low.data_cb = NULL;
    cfg->spic_low.error_cb = NULL;
    cfg->spic_low.cr2 = 0;
    cfg->spic_low.cr1 = SPI_CR1_BR_1 | SPI_CR1_BR_0;
    spiStart(cfg->spid_low, &cfg->spic_low);

    tim15_cfg = cfg;
    rccEnableTIM15(true);
    rccResetTIM15();
    nvicEnableVector(STM32_TIM15_NUMBER, STM32_IRQ_TIM15_PRIORITY);

    if (cfg->blank_period == 0) {
        cfg->blank_period = 100 * cfg->base_divider;
    }

    if (cfg->unblank_period_high[0] == 0) {
        for (size_t i=0; i<cfg->bit_depth; i++) {
            cfg->unblank_period_high[i] = (2<<i)*cfg->base_divider + cfg->offset_correction;
        }
    }

    for (size_t i=0; i<cfg->bit_depth; i++) {
        cfg->unblank_period_high[i] += cfg->front_porch;
    }

    TIM15->CR1 = STM32_TIM_CR1_ARPE;
    TIM15->CR2 = STM32_TIM_CR2_MMS(2);
    TIM15->PSC = cfg->prescaler-1;
    TIM15->ARR = 24; /* arbitrary init value */
    TIM15->CCMR1 = STM32_TIM_CCMR1_OC1PE | STM32_TIM_CCMR1_OC1M(6);
    TIM15->CCER = STM32_TIM_CCER_CC1NE | STM32_TIM_CCER_CC1NP;
    TIM15->CCR1 = cfg->front_porch;
    TIM15->DIER = STM32_TIM_DIER_UIE;
    TIM15->CR1 |= STM32_TIM_CR1_CEN;
    TIM15->BDTR = STM32_TIM_BDTR_MOE;
    TIM15->EGR = STM32_TIM_EGR_UG;

    rccEnableTIM1(true);
    rccResetTIM1();
    TIM1->CR1 = STM32_TIM_CR1_ARPE;
    TIM1->CR2 = 0;
    TIM1->PSC = cfg->prescaler-1;
    TIM1->SMCR = STM32_TIM_SMCR_SMS(4);
    TIM1->CCMR1 = STM32_TIM_CCMR1_OC1PE | STM32_TIM_CCMR1_OC1M(6);
    TIM1->CCMR2 = STM32_TIM_CCMR2_OC3PE | STM32_TIM_CCMR2_OC3M(6);
    TIM1->CCER = STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC1P | STM32_TIM_CCER_CC3E;
    TIM1->BDTR = STM32_TIM_BDTR_MOE;
    TIM1->CR1 |= STM32_TIM_CR1_CEN;

    cfg->p.thread = chThdCreateStatic(cfg->p._wa,
            sizeof(cfg->p._wa), DAZZLE_PRIO_PRL_HCM, HighCurrentModulation, cfg);

}

