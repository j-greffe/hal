#include <msp430.h>
#include "hal_clk_p.h"
#include "hal_timer.h"

#if defined(__MSP430_HAS_TA3__)
static volatile uint16_t* g_tar[HAL_TIMER_QTY] = {
     &TA0R,
     &TA1R
};
static volatile uint16_t* g_tactl[HAL_TIMER_QTY] = {
     &TA0CTL,
     &TA1CTL
};
static volatile uint16_t* g_tacctl[HAL_TIMER_QTY][HAL_TIMER_INT_QTY] = {
    { &TA0CCTL0, &TA0CCTL1, &TA0CCTL2 },
    { &TA1CCTL0, &TA1CCTL1, &TA1CCTL2 }
};
static volatile uint16_t* g_taccr[HAL_TIMER_QTY][HAL_TIMER_INT_QTY] = {
    { &TA0CCR0, &TA0CCR1, &TA0CCR2 },
    { &TA1CCR0, &TA1CCR1, &TA1CCR2 }
};

#else
#error "Please define your timer."
#endif

static hal_timer_t g_timer[HAL_TIMER_QTY][HAL_TIMER_INT_QTY];

void hal_timer_open(hal_timer_hw_t timer, hal_timer_clk_t clock)
{
    if (HAL_TIMER_CLK_32K == clock)
    {
        // ACKL / 8
        hal_clk_set(HAL_CLK_ACLK, HAL_CLK_BASE_ACLK/8);

        // 16 bits, ACLK (32768Hz external oscillator / 8), DIV/4, Continuous mode.
        // --> (2^16)/((32768/8)/4) = 64s before wrap
        // Timer Tick is 1/1024th second
        *(g_tactl[timer]) = (TASSEL_1 | ID_2 | MC_2);
    }
    else if (HAL_TIMER_CLK_SYS == clock)
    {
        // SMCLK / 8
        hal_clk_set(HAL_CLK_SMCLK, HAL_CLK_BASE_MCLK/8);

        // 16 bits, SMCLK (16-26MHz / 8), DIV/8, Continuous mode.
        // --> (2^16)/((2000000)/8) = 0.16~0.26s max before wrap
        // Timer Tick is 1/250000th second
        *(g_tactl[timer]) = (TASSEL_2 | ID_3 | MC_2);
    }

    __enable_interrupt();
}

void hal_timer_close(hal_timer_hw_t timer)
{
    uint16_t reg;

    reg = *(g_tactl[timer]);
    reg &= ~(MC0 | MC1);
    reg |= MC_0;
    *(g_tactl[timer]) = reg;
}

void hal_timer_start(hal_timer_hw_t timer, hal_timer_int_t idx, uint8_t mode, hal_isr_t action, void* param, uint16_t ti)
{
    // Configure timer
    g_timer[timer][idx].time = ti;
    g_timer[timer][idx].mode = mode;
	g_timer[timer][idx].action = action;
	g_timer[timer][idx].param = param;

	if (HAL_TIMER_MODE_WRAP == g_timer[HAL_TIMER_0][idx].mode)
	{
	    // Let timer run continuously. Interrupt will occur at wrap
        // Configure interrupt
        *(g_taccr[timer][idx]) = 0;
        *(g_tacctl[timer][idx]) = (CCIE);
	}
	else
	{
        // Configure interrupt
        *(g_taccr[timer][idx]) = (uint16_t)(ti+(uint16_t)TAR);
        *(g_tacctl[timer][idx]) = (CCIE);
	}
}

void hal_timer_stop(hal_timer_hw_t timer, hal_timer_int_t idx)
{
	*(g_tacctl[timer][idx]) = 0;
	g_timer[timer][idx].mode = HAL_TIMER_MODE_OFF;
}

uint16_t hal_timer_get_time(hal_timer_hw_t timer)
{
    return *(g_tar[timer]);
}

// Timer A0 interrupt service routine
// ISR for CCR0
// high priority
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0_isr(void)
{
    hal_lpm_exit();

    if (HAL_TIMER_MODE_CONTINOUS == g_timer[HAL_TIMER_0][HAL_TIMER_INT_0].mode)
    {
        // Update CCR for next interrupt
        *(g_taccr[HAL_TIMER_0][HAL_TIMER_INT_0]) += g_timer[HAL_TIMER_0][HAL_TIMER_INT_0].time;
    }
    else if (HAL_TIMER_MODE_WRAP == g_timer[HAL_TIMER_0][HAL_TIMER_INT_0].mode)
    {
        // Let timer wrap again
    }
    else
    {
        // Disable interrupt on timer
        *(g_tacctl[HAL_TIMER_0][HAL_TIMER_INT_0]) = 0;
    }

	// Execute action
	if (g_timer[HAL_TIMER_0][HAL_TIMER_INT_0].action)
	{
	    g_timer[HAL_TIMER_0][HAL_TIMER_INT_0].action(g_timer[HAL_TIMER_0][HAL_TIMER_INT_0].param);
	}
}

// Timer A0 interrupt service routine
// ISR for CCR1+
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1_isr(void)
{
    // Get idx, this clears all interrupt flags
    uint8_t idx = (uint8_t)(TA0IV >> 1);

    hal_lpm_exit();

    if (HAL_TIMER_MODE_CONTINOUS == g_timer[HAL_TIMER_0][idx].mode)
    {
        // Update CCR for next interrupt
        *(g_taccr[HAL_TIMER_0][idx]) += g_timer[HAL_TIMER_0][idx].time;
    }
    else if (HAL_TIMER_MODE_WRAP == g_timer[HAL_TIMER_0][idx].mode)
    {
        // Let timer wrap again
    }
    else
    {
        // Disable interrupt on timer
        *(g_tacctl[HAL_TIMER_0][idx]) = 0;
    }

    // Execute action
    if (g_timer[HAL_TIMER_0][idx].action)
    {
        g_timer[HAL_TIMER_0][idx].action(g_timer[HAL_TIMER_0][idx].param);
    }
}
