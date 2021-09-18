#ifndef _HAL_PWM_H_
#define _HAL_PWM_H_

#include <stdint.h>
#include "hal_pwm.h"
#include "hal_gpio.h"

#define PWM_RATIO_MAX       1024

typedef enum {
    HAL_PWM_0, // XXX not usable if kal_timer is used
    HAL_PWM_1,
    HAL_PWM_QTY,
} hal_pwm_t;

int hal_pwm_open(hal_pwm_t pwm, gpio_t io);
int hal_pwm_cfg(hal_pwm_t pwm, uint16_t freq, uint16_t ratio);
int hal_pwm_start(hal_pwm_t pwm);
int hal_pwm_stop(hal_pwm_t pwm);

#endif // _HAL_PWM_H_
