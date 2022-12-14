
#include <math.h>
#include <string.h>
#include <unistd.h>

#include <ch.h>
#include <osal.h>
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


#define BITSPERLONG 32
#define TOP2BITS(x) ((x & (3L << (BITSPERLONG-2))) >> (BITSPERLONG-2))

/* usqrt:
    ENTRY x: unsigned long
    EXIT  returns floor(sqrt(x) * pow(2, BITSPERLONG/2))

    Since the square root never uses more than half the bits
    of the input, we use the other half of the bits to contain
    extra bits of precision after the binary point.

    EXAMPLE
        suppose BITSPERLONG = 32
        then    usqrt(144) = 786432 = 12 * 65536
                usqrt(32) = 370727 = 5.66 * 65536

    NOTES
        (1) change BITSPERLONG to BITSPERLONG/2 if you do not want
            the answer scaled.  Indeed, if you want n bits of
            precision after the binary point, use BITSPERLONG/2+n.
            The code assumes that BITSPERLONG is even.
        (2) This is really better off being written in assembly.
            The line marked below is really a "arithmetic shift left"
            on the double-long value with r in the upper half
            and x in the lower half.  This operation is typically
            expressible in only one or two assembly instructions.
        (3) Unrolling this loop is probably not a bad idea.

    ALGORITHM
        The calculations are the base-two analogue of the square
        root algorithm we all learned in grammar school.  Since we're
        in base 2, there is only one nontrivial trial multiplier.

        Notice that absolutely no multiplications or divisions are performed.
        This means it'll be fast on a wide range of processors.
*/

uint32_t usqrt(uint32_t x)
{
      uint32_t a = 0L;                   /* accumulator      */
      uint32_t r = 0L;                   /* remainder        */
      uint32_t e = 0L;                   /* trial product    */

      int i;

      for (i = 0; i < BITSPERLONG; i++)   /* NOTE 1 */
      {
            r = (r << 2) + TOP2BITS(x); x <<= 2; /* NOTE 2 */
            a <<= 1;
            e = (a << 1) + 1;
            if (r >= e)
            {
                  r -= e;
                  a++;
            }
      }
      return a;
}


static void precalc_modulation(struct high_current_modulation_cfg *cfg) {
    /* transpose brightness values from cfg->val to cfg->data_{high,low} */

    size_t act = cfg->p.writer;

    memset(cfg->p.data_high[act], 0, sizeof(cfg->p.data_high[act]));
    memset(cfg->p.data_low[act], 0, sizeof(cfg->p.data_low[act]));

    for (size_t i=0; i<cfg->channel_count; i++) {
        /* fixme too slow
        float value = powf(cfg->val[i] / (float)0xffff, cfg->gamma);
        uint32_t value_ui = value * 0xffffffff + 0.5f;
        */

        /* approximate gamma */
        /*
        uint16_t v = cfg->val[i];
        uint64_t sqr = usqrt(v);
        uint64_t ssqr = usqrt(sqr>>16);
        uint64_t vsq = ((uint32_t)v)*((uint32_t)v);
        uint32_t value_ui = (((vsq * sqr) >> 24) * ssqr)>>20;
        */
        uint32_t value_ui = cfg->val[i]<<16;

        /* TODO match up channels */
        int map_high = cfg->high_channel_map[i];
        int map_low = cfg->low_channel_map[i];
        uint32_t bit_mask_high = 1<<(map_high%8);
        uint32_t bit_mask_low = 1<<(map_low%8);
        size_t reg_index_high = map_high/8;
        size_t reg_index_low = map_low/8;

        uint32_t high_bit = 0x80000000UL;
        uint32_t low_bit = high_bit / cfg->range_scale;
        for (size_t bit=0; bit<cfg->bit_depth; bit++) {
            if (value_ui > high_bit) {
                cfg->p.data_high[act][cfg->bit_depth-1-bit][reg_index_high] |= bit_mask_high;
                value_ui -= high_bit;
            }
            high_bit >>= 1;
        }
        for (size_t bit=0; bit<cfg->bit_depth; bit++) {
            if (value_ui > low_bit) {
                cfg->p.data_low[act][cfg->bit_depth-1-bit][reg_index_low] |= bit_mask_low;
                value_ui -= low_bit;
            }
            low_bit >>= 1;
        }
    }

    if (!cfg->p.update) {
        cfg->p.writer = cfg->p.ready;
        cfg->p.ready = act;
        cfg->p.update = true;
    }
}

int dbg_num = 0;
uint8_t d0=0, d1=0, d2=0, d3=0;


#  define HSV2RGB(_H, _S, _V, _R, _G, _B) \
    const uint8_t __H = _H; \
    const uint8_t __S = _S; \
    const uint8_t __V = _V; \
    const uint32_t s = (6 * (uint32_t)__H) >> 8;               /* the segment 0..5 (360/60 * [0..255] / 256) */ \
    const uint32_t t = (6 * (uint32_t)__H) & 0xff;             /* within the segment 0..255 (360/60 * [0..255] % 256) */ \
    const uint32_t l = ((uint32_t)__V * (255 - (uint32_t)__S)) >> 8; /* lower level */ \
    const uint32_t rf = ((uint32_t)__V * (uint32_t)__S * t) >> 16;    /* ramp */ \
    switch (s) \
    { \
        case 0: (_R) = (uint8_t)__V;        (_G) = (uint8_t)(l + rf);    (_B) = (uint8_t)l;          break; \
        case 1: (_R) = (uint8_t)(__V - rf);  (_G) = (uint8_t)__V;        (_B) = (uint8_t)l;          break; \
        case 2: (_R) = (uint8_t)l;          (_G) = (uint8_t)__V;        (_B) = (uint8_t)(l + rf);    break; \
        case 3: (_R) = (uint8_t)l;          (_G) = (uint8_t)(__V - rf);  (_B) = (uint8_t)__V;        break; \
        case 4: (_R) = (uint8_t)(l + rf);    (_G) = (uint8_t)l;          (_B) = (uint8_t)__V;        break; \
        case 5: (_R) = (uint8_t)__V;        (_G) = (uint8_t)l;          (_B) = (uint8_t)(__V - rf);  break; \
    }

static THD_FUNCTION(HighCurrentModulation, vcfg) {
    struct high_current_modulation_cfg *cfg = vcfg;

    int j = 0;
    int k = 0;
    while (true) {
        chThdSleepMilliseconds(10);
        //j = (j+17)%0x10000;
        k ++;
        if (k >= 360) {
            k = 0;
            //j += 10; //j = (j+1+j/200+j/2000+j/20000);
            j++;
            if (j >= 8) {
                j = 0;
            }
        }

        /*
        j ++;
        if (j >= 0x0800) {
            j = 0;
            asm volatile ("nop");
        }
        */
        //j = (j+1)%20;
        /*
        for (size_t i=0; i<sizeof(cfg->val)/sizeof(cfg->val[0]); i++) {
            if (i<3) {
                cfg->val[i] = j;
            } else {
                cfg->val[i] = 0;
            }
        }
        */
        memset(cfg->val, 0, sizeof(cfg->val));
        uint8_t r, g, b;
        {
            HSV2RGB(k, 255, 255, r, g, b);
        }
        for (int l=0; l<9; l++) {
            cfg->val[3*l + 0] = r<<j;
            cfg->val[3*l + 1] = g<<j;
            cfg->val[3*l + 2] = b<<j;
        }
        precalc_modulation(cfg);

        /*
        size_t act = cfg->p.writer;
        memset(cfg->p.data_high[act], 0, sizeof(cfg->p.data_high[act]));
        memset(cfg->p.data_low[act], 0, sizeof(cfg->p.data_low[act]));

        for (int i=0; i<4; i++) {
            (j < 0x0400 ? cfg->p.data_high[act][1] : cfg->p.data_low[act][5])[i] = 0xff;
        }
        */

        //for (size_t i=0; i<4; i++) {
        //    cfg->p.data_low[act][7][i] = 127;
        //}
        //cfg->p.data_low[act][7][dbg_num/8] |= 1<<(dbg_num%8);
        //cfg->p.data_low[act][7][0] = d0;
        //cfg->p.data_low[act][7][1] = d1;
        //cfg->p.data_low[act][7][2] = d2;
        //cfg->p.data_low[act][7][3] = d3;

        /*
        if (!cfg->p.update) {
            cfg->p.writer = cfg->p.ready;
            cfg->p.ready = act;
            cfg->p.update = true;
        }
        */

        /* FIXME DEBUG CODE */
        /*
        for (size_t i=0; i<cfg->bit_depth; i++) {
            cfg->unblank_period_high[i] = (2<<i)*cfg->base_divider + cfg->high_offset_correction + cfg->front_porch;
            cfg->unblank_period_low[i] = (2<<i)*cfg->base_divider + cfg->low_offset_correction;
        }
        */

        /*
        size_t act = cfg->p.writer;
        memset(cfg->p.data_high[act], 0x00, sizeof(cfg->p.data_high[0]));
        memset(cfg->p.data_low[act], 0x00, sizeof(cfg->p.data_low[0]));
        if (j>=5) {
            memset(cfg->p.data_low[act][9], 0xff, sizeof(cfg->p.data_low[0][0]));
        } else {
            memset(cfg->p.data_high[act][0], 0xff, sizeof(cfg->p.data_high[0][0]));
        }
        if (!cfg->p.update) {
            cfg->p.writer = cfg->p.ready;
            cfg->p.ready = act;
            cfg->p.update = true;
        }
        */

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

void calculate_gamma_table(struct high_current_modulation_cfg *cfg, float gamma) {
}

static struct high_current_modulation_cfg *tim15_cfg;
OSAL_IRQ_HANDLER(STM32_TIM15_HANDLER) {
    OSAL_IRQ_PROLOGUE();
    GPIOA->BSRR.H.set = 1<<15;
    TIM15->SR = 0;

    static size_t bit_pos_lookup[10] = {0,1,2,3,4,5,6,7,8,9};
    struct high_current_modulation_cfg *cfg = tim15_cfg;
    bool is_blank = cfg->p.is_blank;
    cfg->p.is_blank = !is_blank;
    if (is_blank) {
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

        TIM15->ARR = cfg->unblank_period_high[bit_pos];
        TIM15->CCR1 = cfg->unblank_width_high[bit_pos];
        TIM1->CCR3 = cfg->unblank_period_low[bit_pos];
        TIM1->CCR1 = 0;

        size_t act;
        if (cfg->p.update) {
            act = cfg->p.ready;
            cfg->p.ready = cfg->p.reader;
            cfg->p.reader = act;
            cfg->p.update = false;
        } else {
            act = cfg->p.reader;
        }
        spiStartSendI(cfg->spid_high, (cfg->channel_count+7)/8, cfg->p.data_high[act][bit_pos]);
        spiStartSendI(cfg->spid_low, (cfg->channel_count+7)/8, cfg->p.data_low[act][bit_pos]);

    } else {
        TIM15->ARR = cfg->blank_period;
        TIM15->CCR1 = cfg->blank_period - cfg->front_porch;
        TIM1->CCR3 = 0;
        TIM1->CCR1 = cfg->low_strobe_allowance;
    }

    GPIOA->BSRR.H.clear = 1<<15;
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
    uint16_t old_arr = TIM1->ARR;
    //uint16_t old_ccmr2 = TIM1->CCMR2;
    palSetLine(cfg->tlc_mode_line);
    //TIM1->CCMR2 = (TIM1->CCMR2 & ~STM32_TIM_CCMR2_OC2M_Mask) | STM32_TIM_CCMR2_OC2M(5); /* force high */
    uint8_t data[14];
    for (size_t i=0; i<(cfg->channel_count+15)/16; i++) {
        tlc5922_pack_dot_correction(&cfg->dot_correction[i*16], data);
        spiSend(cfg->spid_low, sizeof(data), data);
    }

    TIM1->ARR = 48; /* Generate a train of strobe pulses */
    TIM1->CCR1 = 24; /* Generate a train of strobe pulses */
    chThdSleepMilliseconds(5);
    palClearLine(cfg->tlc_mode_line);
    TIM1->ARR = old_arr;
    //TIM1->CCMR2 = old_ccmr2;
}

void dazzle_high_current_modulation_run(struct high_current_modulation_cfg *cfg) {
    cfg->p.is_blank = false;
    cfg->p.bit_pos = 0;
    cfg->p.reader = 0;
    cfg->p.ready = 1;
    cfg->p.writer = 2;
    cfg->p.update = 0;
    memset(cfg->p.data_low, 0, sizeof(cfg->p.data_low));
    memset(cfg->p.data_high, 0, sizeof(cfg->p.data_high));

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
    
    if (cfg->low_strobe_allowance == 0) {
        cfg->low_strobe_allowance = 2000;
    }

    if (cfg->unblank_period_high[0] == 0) {
        for (size_t i=0; i<cfg->bit_depth; i++) {
            int power = (2<<i)*cfg->base_divider;
            if (power < cfg->min_unblank_period) {
                cfg->unblank_period_high[i] = cfg->min_unblank_period;
            } else {
                cfg->unblank_period_high[i] = power;
            }
            cfg->unblank_width_high[i] = power + cfg->high_offset_correction - cfg->front_porch;
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
    TIM15->CCMR1 = STM32_TIM_CCMR1_OC1PE | STM32_TIM_CCMR1_OC1M(7); /* shift register / high strobe */
    TIM15->CCER = STM32_TIM_CCER_CC1NE;
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
    TIM1->ARR = 0xffff;
    TIM1->CCMR1 = STM32_TIM_CCMR1_OC1PE | STM32_TIM_CCMR1_OC1M(6); /* OC1: TLC / low strobe */
    TIM1->CCMR2 = STM32_TIM_CCMR2_OC3PE | STM32_TIM_CCMR2_OC3M(7); /* OC3: TLC / low blank AND SR clear */
    TIM1->CCER = STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC1P | STM32_TIM_CCER_CC3E;
    TIM1->BDTR = STM32_TIM_BDTR_MOE;
    TIM1->CR1 |= STM32_TIM_CR1_CEN;

    tlc5922_load_dot_correction(cfg);

    nvicEnableVector(STM32_TIM15_NUMBER, STM32_IRQ_TIM15_PRIORITY);

    cfg->p.thread = chThdCreateStatic(cfg->p._wa,
            sizeof(cfg->p._wa), DAZZLE_PRIO_PRL_HCM, HighCurrentModulation, cfg);

}

