#ifndef _HAL_CLK_P_H_
#define _HAL_CLK_P_H_

// Private include

#include <stdint.h>

#define HAL_CLK_BASE_ACLK   (32768) // Hz
#define HAL_CLK_BASE_MCLK   (16000000) // Hz (16MHz to 26MHz in reality)

typedef enum {
    HAL_CLK_TACLK,
    HAL_CLK_ACLK,
    HAL_CLK_SMCLK,
    HAL_CLK_INCLK,
    HAL_CLK_MCLK,
} hal_clk_type_t;

void hal_clk_set(hal_clk_type_t clock, uint32_t frequency);
uint32_t hal_clk_get(hal_clk_type_t clock);

#endif // _HAL_CLK_P_H_
