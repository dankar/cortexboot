#include "pll0.h"
#include "inc/LPC17xx.h"
#include "hdr/hdr_sc.h"

static void flash_latency(uint32_t frequency)
{
        uint32_t wait_states;

        wait_states = frequency / 20000000;

        if (wait_states > 5)
                wait_states = 5;

        LPC_SC->FLASHCFG = ((wait_states - 1) << LPC_SC_FLASHCFG_FLASHTIM_bit) | LPC_SC_FLASHCFG_RESERVED_value;
}

void pll0_feed(void)
{
        LPC_SC->PLL0FEED = LPC_SC_PLL0FEED_FIRST;
        LPC_SC->PLL0FEED = LPC_SC_PLL0FEED_SECOND;
}

uint32_t pll0_start()
{
        uint32_t prediv, mul, corediv, fcco, core_frequency;

	uint32_t crystal = 25000000;

        LPC_SC_SCS_OSCRANGE_bb = 1;

        LPC_SC_SCS_OSCEN_bb = 1;                                // enable main oscillator

        flash_latency(120000000);                               // set flash latency

        while (LPC_SC_SCS_OSCSTAT_bb == 0);             // wait for main oscillator to start up

        LPC_SC->CLKSRCSEL = LPC_SC_CLKSRCSEL_CLKSRC_MAIN;       // set main oscillator as PLL0 clocksource

	prediv = 5;
	mul = 48;
	corediv = 4;

        LPC_SC->PLL0CFG = ((prediv - 1) << LPC_SC_PLL0CFG_NSEL0_bit) | ((mul - 1) << LPC_SC_PLL0CFG_MSEL0_bit);       // set NSEL0 and MSEL0
        pll0_feed();                                                    // validate change in PLL0CFG
        LPC_SC_PLL0CON_PLLE0_bb = 1;                    // enable PLL0
        pll0_feed();                                                    // validate change in PLL0CON
        LPC_SC->CCLKCFG = (corediv - 1) << LPC_SC_CCLKCFG_CCLKSEL_bit;     // set core clock divider CCLKSEL

        while (LPC_SC_PLL0STAT_PLOCK0_bb == 0); // wait for PLL0 lock

        LPC_SC_PLL0CON_PLLC0_bb = 1;                    // connect PLL0 as clock source
        pll0_feed();                                                    // validate connection

        return 2*mul*crystal/prediv/corediv;
}
