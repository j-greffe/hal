#ifndef _HAL_IRQ_H_
#define _HAL_IRQ_H_

#include <stdint.h>
#include "hal.h"
#include "hal_gpio.h"

#define HAL_IRQ_FALLING		(1)
#define HAL_IRQ_RISING		(0)

void hal_irq_critical_enter(void);
void hal_irq_critical_exit(void);

void hal_irq_set(gpio_t io, uint8_t edge, hal_isr_t action, void* param);
void hal_irq_en(gpio_t io);
void hal_irq_dis(gpio_t io);
uint8_t hal_irq_get(gpio_t io);
uint8_t hal_irq_get_port(gpio_t io);
void hal_irq_clr(gpio_t io);
void hal_irq_clr_port(gpio_t io);

#endif
