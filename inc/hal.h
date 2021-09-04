#ifndef _HAL_H_
#define _HAL_H_

typedef void(*hal_isr_t)(void*);

#include "hal_adc.h"
#include "hal_clk.h"
#include "hal_gpio.h"
#include "hal_irq.h"
#include "hal_lpm.h"
#include "hal_pwm.h"
#include "hal_sys.h"
#include "hal_timer.h"
#include "hal_wdg.h"

typedef struct {
    bool xtal_32k;
} hal_hwcfg_t;

void hal_open(const hal_hwcfg_t* hwcfg);

#endif
