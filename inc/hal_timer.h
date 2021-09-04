#ifndef _HAL_TIMER_H_
#define _HAL_TIMER_H_

#include <stdint.h>
#include "hal.h"

typedef enum {
    HAL_TIMER_0,
    HAL_TIMER_1,
    HAL_TIMER_QTY,
} hal_timer_hw_t;

typedef enum {
    HAL_TIMER_INT_0,
    HAL_TIMER_INT_1,
    HAL_TIMER_INT_2,
    HAL_TIMER_INT_QTY,
} hal_timer_int_t;

typedef enum {
    HAL_TIMER_CLK_32K,
    HAL_TIMER_CLK_SYS,
    HAL_TIMER_CLK_QTY,
} hal_timer_clk_t;

enum {
    HAL_TIMER_MODE_OFF,
    HAL_TIMER_MODE_ONE_SHOT,
    HAL_TIMER_MODE_CONTINOUS,
    HAL_TIMER_MODE_WRAP,
};

typedef struct
{
    uint16_t time;
    uint8_t mode;
    hal_isr_t action;
    void* param;
} hal_timer_t;

void hal_timer_open(hal_timer_hw_t timer, hal_timer_clk_t clock);
void hal_timer_close(hal_timer_hw_t timer);
void hal_timer_start(hal_timer_hw_t timer, hal_timer_int_t idx, uint8_t mode, hal_isr_t action, void* param, uint16_t ti);
void hal_timer_stop(hal_timer_hw_t timer, hal_timer_int_t idx);
uint16_t hal_timer_get_time(hal_timer_hw_t timer);



#endif // _HAL_TIMER_H_
