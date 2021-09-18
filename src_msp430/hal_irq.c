#include <msp430.h>

#include "hal.h"
#include "hal_irq.h"

static volatile uint8_t* g_pies[] = { &P1IES, &P2IES };
static volatile uint8_t* g_pie[] = { &P1IE, &P2IE };
static volatile uint8_t* g_pifg[] = { &P1IFG, &P2IFG };

typedef struct
{
	hal_isr_t action;
	void* param;
} hal_irq_t;

#define HAL_IRQ_NB  (8*(sizeof(g_pies)/sizeof(uint8_t*)))
static hal_irq_t g_hal_irq[HAL_IRQ_NB];
static volatile uint32_t g_critical = 0;

void hal_irq_critical_enter(void)
{
    if (!g_critical++)
    {
        __disable_interrupt();
    }
}

void hal_irq_critical_exit(void)
{
    if (!(--g_critical))
    {
        __enable_interrupt();
    }
}

void hal_irq_set(gpio_t io, uint8_t edge, hal_isr_t action, void* param)
{
    if (HAL_IO_NONE_BYTE == io.byte) return;

	g_hal_irq[((io.bf.port)*8)+io.bf.pin].action = action;
	g_hal_irq[((io.bf.port)*8)+io.bf.pin].param = param;

	// disable irq
	hal_irq_dis(io);
	// clear flags
	hal_irq_clr_port(io);

	// Clear register
	*(g_pies[io.bf.port]) &= (uint8_t)(~(1 << io.bf.pin));
	// Select edge
	*(g_pies[io.bf.port]) |= (uint8_t)(edge << io.bf.pin);
}

void hal_irq_en(gpio_t io)
{
    if (HAL_IO_NONE_BYTE == io.byte) return;

	hal_irq_clr_port(io);
	*(g_pie[io.bf.port]) |= (uint8_t)(1 << io.bf.pin);
}

void hal_irq_dis(gpio_t io)
{
    if (HAL_IO_NONE_BYTE == io.byte) return;

	*(g_pie[io.bf.port]) &= (uint8_t)(~(1 << io.bf.pin));
}

uint8_t hal_irq_get(gpio_t io)
{
    if (HAL_IO_NONE_BYTE == io.byte) return 0;

	return (*(g_pifg[io.bf.port]) & (uint8_t)(1 << io.bf.pin));
}

uint8_t hal_irq_get_port(gpio_t io)
{
    if (HAL_IO_NONE_BYTE == io.byte) return 0;

	return (*(g_pifg[io.bf.port]));
}

void hal_irq_clr(gpio_t io)
{
    if (HAL_IO_NONE_BYTE == io.byte) return;

	*(g_pifg[io.bf.port]) &= (uint8_t)(~(1 << io.bf.pin));
}

void hal_irq_clr_port(gpio_t io)
{
    if (HAL_IO_NONE_BYTE == io.byte) return;

	*(g_pifg[io.bf.port]) = 0x00;
}


#pragma vector=PORT1_VECTOR
__interrupt void Port1_isr(void)
{
	uint8_t i;
	gpio_t io = HAL_IO_PACK(1,0);

	hal_lpm_exit();

	// check interrupt flags
	for (i = 8; i > 0; i--)
	{
	    io.bf.pin = i-1;
		if (hal_irq_get(io))
		{
			// do action
		    if (g_hal_irq[i-1].action)
		    {
		        g_hal_irq[i-1].action(g_hal_irq[i-1].param);
		    }
		}
	}
	// clear flags
	hal_irq_clr_port(io);
}

#pragma vector=PORT2_VECTOR
__interrupt void Port2_isr(void)
{
	uint8_t i;
	gpio_t io = HAL_IO_PACK(2,0);

    hal_lpm_exit();

	// check interrupt flags
    for (i = 8; i > 0; i--)
    {
        io.bf.pin = i-1;
        if (hal_irq_get(io))
        {
            // do action
            if (g_hal_irq[i+7].action)
            {
                g_hal_irq[i+7].action(g_hal_irq[i+7].param);
            }
        }
    }
	// clear flags
	hal_irq_clr_port(io);
}
