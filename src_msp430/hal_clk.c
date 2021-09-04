#include <msp430.h>
#include <stdint.h>
#include "hal_clk.h"
#include "hal_clk_p.h"

//DCO Frequency
//over recommended ranges of supply voltage and operating free-air temperature (unless otherwise noted)
//PARAMETER     TEST                    CONDITIONS                       VCC   MIN         TYP         MAX     UNIT
//fDCO(0,0)     DCO frequency (0, 0)    RSELx = 0,  DCOx = 0, MODx = 0   3 V   0.06                    0.14    MHz
//fDCO(0,3)     DCO frequency (0, 3)    RSELx = 0,  DCOx = 3, MODx = 0   3 V   0.07                    0.17    MHz
//fDCO(1,3)     DCO frequency (1, 3)    RSELx = 1,  DCOx = 3, MODx = 0   3 V               0.15                MHz
//fDCO(2,3)     DCO frequency (2, 3)    RSELx = 2,  DCOx = 3, MODx = 0   3 V               0.21                MHz
//fDCO(3,3)     DCO frequency (3, 3)    RSELx = 3,  DCOx = 3, MODx = 0   3 V               0.30                MHz
//fDCO(4,3)     DCO frequency (4, 3)    RSELx = 4,  DCOx = 3, MODx = 0   3 V               0.41                MHz
//fDCO(5,3)     DCO frequency (5, 3)    RSELx = 5,  DCOx = 3, MODx = 0   3 V               0.58                MHz
//fDCO(6,3)     DCO frequency (6, 3)    RSELx = 6,  DCOx = 3, MODx = 0   3 V   0.54                    1.06    MHz
//fDCO(7,3)     DCO frequency (7, 3)    RSELx = 7,  DCOx = 3, MODx = 0   3 V   0.80                    1.50    MHz --> At boot
//fDCO(8,3)     DCO frequency (8, 3)    RSELx = 8,  DCOx = 3, MODx = 0   3 V               1.6                 MHz
//fDCO(9,3)     DCO frequency (9, 3)    RSELx = 9,  DCOx = 3, MODx = 0   3 V               2.3                 MHz
//fDCO(10,3)    DCO frequency (10, 3)   RSELx = 10, DCOx = 3, MODx = 0   3 V               3.4                 MHz
//fDCO(11,3)    DCO frequency (11, 3)   RSELx = 11, DCOx = 3, MODx = 0   3 V               4.25                MHz
//fDCO(12,3)    DCO frequency (12, 3)   RSELx = 12, DCOx = 3, MODx = 0   3 V   4.30                    7.30    MHz
//fDCO(13,3)    DCO frequency (13, 3)   RSELx = 13, DCOx = 3, MODx = 0   3 V   6.00        7.8         9.60    MHz
//fDCO(14,3)    DCO frequency (14, 3)   RSELx = 14, DCOx = 3, MODx = 0   3 V   8.60                    13.9    MHz
//fDCO(15,3)    DCO frequency (15, 3)   RSELx = 15, DCOx = 3, MODx = 0   3 V   12.0                    18.5    MHz
//fDCO(15,7)    DCO frequency (15, 7)   RSELx = 15, DCOx = 7, MODx = 0   3 V   16.0                    26.0    MHz

typedef struct {
    bool xtal_32k;
    uint32_t freq_mclk;
    uint32_t freq_smclk;
    uint32_t freq_aclk;
} hal_clk_t;

static volatile hal_clk_t g_clock;

void hal_clk_open(bool xtal_32k)
{
    uint8_t reg;

    g_clock.xtal_32k = xtal_32k;

    // MOD_0. DCO_7.
    reg = DCOCTL;
    reg &= ~((MOD0 | MOD1 | MOD2 | MOD3 | MOD4) | (DCO0 | DCO1 | DCO2));
    reg |= (0) | (DCO0 | DCO1 | DCO2);
    DCOCTL = reg;

    // XT2 off. LFXT1 low freq. ACLK/8. RSEL_15
    reg = BCSCTL1;
    reg &= ~((XT2OFF) | (XTS) | (DIVA0 | DIVA1) | (RSEL0 | RSEL1 | RSEL2 | RSEL3));
    reg |= (0) | (0) | (DIVA_3) | (RSEL0 | RSEL1 | RSEL2 | RSEL3);
    BCSCTL1 = reg;

    // MCLK from DCOCLK. MCLK/1. SMCLK from DCOCLK. SMCLK/1. DCO internal resistor.
    reg = BCSCTL2;
    reg &= ~((SELM0 | SELM1) | (DIVM0 | DIVM1) | (SELS) | (DIVS0 | DIVS1));
    reg |= (SELM_0) | (DIVM_0) | (0) | (DIVS_0);
    BCSCTL2 = reg;

    if (g_clock.xtal_32k)
    {
        // 32768-Hz crystal on LFXT1 (XTS=0). Xcap 12.5pF
        reg = BCSCTL3;
        reg &= ~((XT2S0 | XT2S1) | (LFXT1S0 | LFXT1S1) | (XCAP0 | XCAP1));
        reg |= (XT2S_0) | (LFXT1S_0) | (XCAP_2);
        BCSCTL3 = reg;

        // Wait for fault bit to clear
        while (BCSCTL3 & LFXT1OF);

        g_clock.freq_aclk = HAL_CLK_BASE_ACLK/8;
    }
    else
    {
        g_clock.freq_aclk = 0;
    }

    g_clock.freq_mclk   = HAL_CLK_BASE_MCLK;
    g_clock.freq_smclk  = HAL_CLK_BASE_MCLK;
}

void hal_clk_set(hal_clk_type_t clock, uint32_t frequency)
{
    uint8_t reg;

    switch (clock)
    {
    case HAL_CLK_TACLK:
        // TODO
        break;
    case HAL_CLK_ACLK:

        if (!g_clock.xtal_32k)
        {
            return;
        }

        // Set ACLK divider
        reg = BCSCTL1;
        reg &= ~(DIVA0 | DIVA1);
        if      (HAL_CLK_BASE_ACLK/2 < frequency) { reg |= (DIVA_0); g_clock.freq_aclk = HAL_CLK_BASE_ACLK/1; }
        else if (HAL_CLK_BASE_ACLK/4 < frequency) { reg |= (DIVA_1); g_clock.freq_aclk = HAL_CLK_BASE_ACLK/2; }
        else if (HAL_CLK_BASE_ACLK/8 < frequency) { reg |= (DIVA_2); g_clock.freq_aclk = HAL_CLK_BASE_ACLK/4; }
        else                                      { reg |= (DIVA_3); g_clock.freq_aclk = HAL_CLK_BASE_ACLK/8; }
        BCSCTL1 = reg;

        // Wait for fault bit to clear
        while (BCSCTL3 & LFXT1OF);

        break;
    case HAL_CLK_MCLK:
        // TODO
        break;
    case HAL_CLK_SMCLK:
        // Set SMCLK divider
        reg = BCSCTL2;
        reg &= ~(DIVS0 | DIVS1);
        if      (g_clock.freq_smclk/2 < frequency) { reg |= (DIVS_0); g_clock.freq_smclk = HAL_CLK_BASE_MCLK/1; }
        else if (g_clock.freq_smclk/4 < frequency) { reg |= (DIVS_1); g_clock.freq_smclk = HAL_CLK_BASE_MCLK/2; }
        else if (g_clock.freq_smclk/8 < frequency) { reg |= (DIVS_2); g_clock.freq_smclk = HAL_CLK_BASE_MCLK/4; }
        else                                       { reg |= (DIVS_3); g_clock.freq_smclk = HAL_CLK_BASE_MCLK/8; }
        BCSCTL2 = reg;
        break;
    case HAL_CLK_INCLK:
        // TODO
        break;
    default:
        break;
    }
}

uint32_t hal_clk_get(hal_clk_type_t clock)
{
    switch (clock)
    {
    case HAL_CLK_MCLK:
        return g_clock.freq_mclk;
    case HAL_CLK_SMCLK:
        return g_clock.freq_smclk;
    case HAL_CLK_ACLK:
        return g_clock.freq_aclk;
    default:
        return 0;
    }
}
