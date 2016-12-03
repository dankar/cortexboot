#include "pll0.h"
#include "inc/LPC17xx.h"
#include "hdr/hdr_sc.h"

static void flash_latency(uint32_t frequency)
{
        uint32_t wait_states;

        wait_states = frequency / 20000000;             // 1 CLK per 20MHz

        if (wait_states > 5)                                    // 5 CLKs is the highest reasonable value, works for
                wait_states = 5;

        LPC_SC->FLASHCFG = ((wait_states - 1) << LPC_SC_FLASHCFG_FLASHTIM_bit) | LPC_SC_FLASHCFG_RESERVED_value;
}

static void pll0_feed(void)
{
        LPC_SC->PLL0FEED = LPC_SC_PLL0FEED_FIRST;
        LPC_SC->PLL0FEED = LPC_SC_PLL0FEED_SECOND;
}

uint32_t pll0_start(uint32_t crystal, uint32_t frequency)
{
        uint32_t prediv, mul, corediv, fcco, core_frequency;
        uint32_t best_prediv = 0, best_mul = 0, best_corediv = 0, best_core_frequency = 0;

        if (crystal > 20000000)                                 // change OSCRANGE for crystals over 20MHz
                LPC_SC_SCS_OSCRANGE_bb = 1;

        LPC_SC_SCS_OSCEN_bb = 1;                                // enable main oscillator

        flash_latency(frequency);                               // set flash latency

        while (LPC_SC_SCS_OSCSTAT_bb == 0);             // wait for main oscillator to start up

        LPC_SC->CLKSRCSEL = LPC_SC_CLKSRCSEL_CLKSRC_MAIN;       // set main oscillator as PLL0 clocksource

        for (prediv = 1; prediv <= 32; prediv++)        // NSEL0 in [1; 32]
                for (mul = 6; mul <= 512; mul++)                // MSEL0 in [5; 512]
                {
                        fcco = 2 * mul * crystal / prediv;      // calculate PLL output frequency

                        if ((fcco < 275000000) || (fcco > 550000000))   // skip invalid settings - fcco must be in [275M; 550M]
                                continue;

                        for (corediv = 1; corediv <= 256; corediv++)    // CCLKSEL in [1; 256]
                        {
                                core_frequency = fcco / corediv;        // calculate core frequency

                                if (core_frequency > frequency)         // skip frequencies above desired value
                                        continue;

                                if (core_frequency > best_core_frequency)       // is this configuration better than previous one?
                                {
                                        best_core_frequency = core_frequency;   // yes - save values
                                        best_prediv = prediv;
                                        best_mul = mul;
                                        best_corediv = corediv;

                                        if (core_frequency == frequency)        // is this configuration "perfect"?
                                                break;                          // yes - skip further search
                                }
                        }
                }
        LPC_SC->PLL0CFG = ((best_prediv - 1) << LPC_SC_PLL0CFG_NSEL0_bit) | ((best_mul - 1) << LPC_SC_PLL0CFG_MSEL0_bit);       // set NSEL0 and MSEL0
        pll0_feed();                                                    // validate change in PLL0CFG
        LPC_SC_PLL0CON_PLLE0_bb = 1;                    // enable PLL0
        pll0_feed();                                                    // validate change in PLL0CON
        LPC_SC->CCLKCFG = (best_corediv - 1) << LPC_SC_CCLKCFG_CCLKSEL_bit;     // set core clock divider CCLKSEL

        while (LPC_SC_PLL0STAT_PLOCK0_bb == 0); // wait for PLL0 lock

        LPC_SC_PLL0CON_PLLC0_bb = 1;                    // connect PLL0 as clock source
        pll0_feed();                                                    // validate connection

        return best_core_frequency;
}
