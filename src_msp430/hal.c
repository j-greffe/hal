#include "hal.h"

void hal_open(const hal_hwcfg_t* hwcfg)
{
    // Init clocks
    hal_clk_open(hwcfg->xtal_32k);

    // Open timer 0 for kal_timer
    hal_timer_open(HAL_TIMER_0, (hwcfg->xtal_32k)? HAL_TIMER_CLK_32K: HAL_TIMER_CLK_SYS);

    // Open timer 1 for hal_pwm
    hal_timer_open(HAL_TIMER_1, HAL_TIMER_CLK_SYS);
}
