#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "hal_clk_p.h"
#include "hal_gpio.h"
#include "hal_pwm.h"

static volatile uint16_t* g_tactl[HAL_PWM_QTY] = {
     &TA0CTL,
#if defined(__MSP430_HAS_TA3__)
     &TA1CTL,
#endif
};

static volatile uint16_t* g_tacctl[HAL_PWM_QTY][3] = {
    { &TA0CCTL0, &TA0CCTL1, &TA0CCTL2 },
#if defined(__MSP430_HAS_TA3__)
    { &TA1CCTL0, &TA1CCTL1, &TA1CCTL2 },
#endif
};

static volatile uint16_t* g_taccr[HAL_PWM_QTY][3] = {
    { &TA0CCR0, &TA0CCR1, &TA0CCR2 },
#if defined(__MSP430_HAS_TA3__)
    { &TA1CCR0, &TA1CCR1, &TA1CCR2 },
#endif
};

static bool g_setup[HAL_PWM_QTY] = { 0 };
static bool g_conf[HAL_PWM_QTY] = { 0 };

int hal_pwm_open(hal_pwm_t pwm, gpio_t io)
{
    if (HAL_IO_NONE_BYTE == io.byte) return -1;

    // 16 bits, SMCLK, DIV/1
    *(g_tactl[pwm]) = (TASSEL_2 | ID_0);

	// Reset/Set for CCR1
    *(g_tacctl[pwm][1]) = (OUTMOD_7);

	// Pin in ALT mode
	hal_gpio_cfg(io, HAL_IO_ALT | HAL_IO_OUT);

	g_setup[pwm] = true;

	return 0;
}

int hal_pwm_cfg(hal_pwm_t pwm, uint16_t freq, uint16_t ratio)
{
    uint32_t f;
    uint32_t r;

    if (!g_setup[pwm])
    {
        g_conf[pwm] = false;
        return -1;
    }

    // Get clock
    f = hal_clk_get(HAL_CLK_SMCLK);

    // Divide clock by wanted frequency to get counter value
    f /= freq;

    // Min frequency is SMCLK/0xFFFF
    if (f > 0xFFFF)
    {
        // Frequency too low for this clock speed
        g_conf[pwm] = false;
        return -1;
    }

    // Set counter
    *(g_taccr[pwm][0]) = f;

	// Set duty cycle
	r = ratio;
	r *= f;
	r /= PWM_RATIO_MAX;
	*(g_taccr[pwm][1]) = r;

	// PWM is configured
	g_conf[pwm] = true;

	return 0;
}

int hal_pwm_start(hal_pwm_t pwm)
{
    if (!g_setup[pwm] || !g_conf[pwm]) return -1;

    hal_pwm_stop(pwm);
    *(g_tactl[pwm]) |= MC_1; // UP mode

	return 0;
}

int hal_pwm_stop(hal_pwm_t pwm)
{
    if (!g_setup[pwm] || !g_conf[pwm]) return -1;

    *(g_tactl[pwm]) &= ~(MC0 | MC1);

	return 0;
}
