
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

static void tlc5922_load_dot_correction(struct high_current_modulation_cfg *cfg);

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
    memset(cfg->p.data_high, 0, sizeof(cfg->p.data_high));
    memset(cfg->p.data_low, 0xff, sizeof(cfg->p.data_low));

    for (size_t i=0; i<cfg->channel_count; i++) {
        /* fixme too slow
        float value = powf(cfg->val[i] / (float)0xffff, cfg->gamma);
        uint32_t value_ui = value * 0xffffffff + 0.5f;
        */
        uint32_t value_ui = cfg->val[i]<<16;

        /* TODO match up channels */
        int map_high = cfg->high_channel_map[i];
        int map_low = cfg->low_channel_map[i];
        size_t bit_mask_high = 1<<(map_high%8);
        size_t bit_mask_low = ~(1<<(map_low%8));
        size_t reg_index_high = map_high/8;
        size_t reg_index_low = map_low/8;

        uint32_t high_bit = 0x80000000UL;
        uint32_t low_bit = high_bit / cfg->range_scale;
        for (size_t bit=0; bit<cfg->bit_depth; bit++) {
            if (value_ui > high_bit) {
                cfg->p.data_high[cfg->bit_depth-1-bit][reg_index_high] |= bit_mask_high;
                value_ui -= high_bit;
            } else if (value_ui > low_bit) {
                cfg->p.data_low[cfg->bit_depth-1-bit][4-reg_index_low] &= bit_mask_low;
                value_ui -= low_bit;
            }
            high_bit >>= 1;
            low_bit >>= 1;
        }
    }
}

static THD_FUNCTION(HighCurrentModulation, vcfg) {
    struct high_current_modulation_cfg *cfg = vcfg;

    int j = 0;
    while (true) {
        chThdSleepMilliseconds(2);
        j = (j+7)%0x10000;
        for (size_t i=0; i<sizeof(cfg->val)/sizeof(cfg->val[0]); i++) {
            cfg->val[i] = (i == 28) ? j : 0;
        }
        precalc_modulation(cfg);

        //memset(cfg->p.data_high, 0x00, sizeof(cfg->p.data_high));
        //memset(cfg->p.data_low, 0x00, sizeof(cfg->p.data_low)); /* TODO data_low is inverted */
        //memset(cfg->dot_correction, j, sizeof(cfg->dot_correction));

        //nvicDisableVector(STM32_TIM15_NUMBER);
        //tlc5922_load_dot_correction(cfg);
        //nvicEnableVector(STM32_TIM15_NUMBER, STM32_IRQ_TIM15_PRIORITY);
        /*
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
        */
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
        spiStartSendI(cfg->spid_low, cfg->channel_count/8, cfg->p.data_low[bit_pos]);
        TIM15->ARR = cfg->unblank_period_high[bit_pos];
        TIM1->CCR3 = cfg->unblank_period_low[bit_pos] - 24; /* FIXME debug code */
        TIM1->CCR1 = 0;

    } else {
        palClearLine(cfg->sr_clear_line);
        TIM15->ARR = cfg->blank_period;
        TIM1->CCR3 = 0;
        TIM1->CCR1 = cfg->high_strobe_allowance;
    }

    OSAL_IRQ_EPILOGUE();
}

static void tlc5922_pack_dot_correction(const uint8_t input[16], uint8_t output[14]) {
    output[ 0] = (input[15]<<1) | (input[14]>>6);
    output[ 1] = (input[14]<<2) | (input[13]>>5);
    output[ 2] = (input[13]<<3) | (input[12]>>4);
    output[ 3] = (input[12]<<4) | (input[11]>>3);
    output[ 4] = (input[11]<<5) | (input[10]>>2);
    output[ 5] = (input[10]<<6) | (input[ 9]>>1);
    output[ 6] = (input[ 9]<<7) | (input[ 8]>>0);

    output[ 7] = (input[ 7]<<1) | (input[ 6]>>6);
    output[ 8] = (input[ 6]<<2) | (input[ 5]>>5);
    output[ 9] = (input[ 5]<<3) | (input[ 4]>>4);
    output[10] = (input[ 4]<<4) | (input[ 3]>>3);
    output[11] = (input[ 3]<<5) | (input[ 2]>>2);
    output[12] = (input[ 2]<<6) | (input[ 1]>>1);
    output[13] = (input[ 1]<<7) | (input[ 0]>>0);
}

static void tlc5922_load_dot_correction(struct high_current_modulation_cfg *cfg) {
    palSetLine(cfg->tlc_mode_line);
    uint8_t data[(DAZZLE_HCM_MAX_REGISTERS+1)/2*14];
    for (size_t i=0; i<(cfg->channel_count+15)/16; i++) {
        tlc5922_pack_dot_correction(&cfg->dot_correction[i*16], data);
        spiSend(cfg->spid_low, sizeof(data), data);
    }

    TIM1->ARR = 48; /* Generate a train of strobe pulses */
    TIM1->CCR1 = 24; /* Generate a train of strobe pulses */
    chThdSleepMilliseconds(5);
    palClearLine(cfg->tlc_mode_line);
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

    if (cfg->blank_period == 0) {
        cfg->blank_period = 100 * cfg->base_divider;
    }
    
    if (cfg->high_strobe_allowance == 0) {
        cfg->high_strobe_allowance = cfg->blank_period - 20;
    }

    if (cfg->unblank_period_high[0] == 0) {
        for (size_t i=0; i<cfg->bit_depth; i++) {
            cfg->unblank_period_high[i] = (2<<i)*cfg->base_divider + cfg->high_offset_correction;
            cfg->unblank_period_low[i] = (2<<i)*cfg->base_divider + cfg->low_offset_correction;
        }
    }

    for (size_t i=0; i<cfg->bit_depth; i++) {
        cfg->unblank_period_high[i] += cfg->front_porch;
    }

    if (cfg->dot_correction[0] == 0) {
        for (size_t i=0; i<sizeof(cfg->dot_correction); i++) {
            cfg->dot_correction[i] = 127;
        }
    }

    tim15_cfg = cfg;
    rccEnableTIM15(true);
    rccResetTIM15();
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
    TIM1->CCR1 = 24;
    TIM1->CCMR1 = STM32_TIM_CCMR1_OC1PE | STM32_TIM_CCMR1_OC1M(6); /* OC1: strobe */
    TIM1->CCMR2 = STM32_TIM_CCMR2_OC3PE | STM32_TIM_CCMR2_OC3M(6); /* OC3: blank */
    TIM1->CCER = STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC1P | STM32_TIM_CCER_CC3E;
    TIM1->BDTR = STM32_TIM_BDTR_MOE;
    TIM1->CR1 |= STM32_TIM_CR1_CEN;

    tlc5922_load_dot_correction(cfg);

    nvicEnableVector(STM32_TIM15_NUMBER, STM32_IRQ_TIM15_PRIORITY);

    cfg->p.thread = chThdCreateStatic(cfg->p._wa,
            sizeof(cfg->p._wa), DAZZLE_PRIO_PRL_HCM, HighCurrentModulation, cfg);

}

