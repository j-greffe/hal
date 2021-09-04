#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "hal_clk_p.h"
#include "hal_gpio.h"
#include "hal_pwm.h"

#if defined(__MSP430G2553)

static bool g_setup = false;
static bool g_conf = false;

int hal_pwm_A1_open(gpio_t io)
{
    if (HAL_IO_NONE_BYTE == io.byte) return -1;

    // 16 bits, SMCLK, DIV/1
	TA1CTL = (TASSEL_2 | ID_0);

	// Reset/Set for CCR1
	TA1CCTL1 = (OUTMOD_7);

	// Pin in ALT mode
	hal_gpio_cfg(io, HAL_IO_ALT | HAL_IO_OUT);

	g_setup = true;

	return 0;
}

int hal_pwm_A1_cfg(uint16_t freq, uint16_t ratio)
{
    uint32_t f;
    uint32_t r;

    if (!g_setup)
    {
        g_conf = false;
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
        g_conf = false;
        return -1;
    }

    // Set counter
	TA1CCR0 = f;

	// Set duty cycle
	r = ratio;
	r *= f;
	r /= PWM_RATIO_MAX;
	TA1CCR1 = r;

	// PWM is configured
	g_conf = true;

	return 0;
}

int hal_pwm_A1_start(void)
{
    if (!g_setup || !g_conf) return -1;

    hal_pwm_A1_stop();
	TA1CTL |= MC_1; // UP mode

	return 0;
}

int hal_pwm_A1_stop(void)
{
    if (!g_setup || !g_conf) return -1;

	TA1CTL &= ~(MC0 | MC1);

	return 0;
}

#endif
