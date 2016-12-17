#include "iap.h"
#include "inc/LPC17xx.h"
#include "hdr/hdr_sc.h"
#include "pll0.h"
#include "common.h"

#define IAP_LOCATION 0x1FFF1FF1

typedef void (*IAP)(unsigned int[], unsigned int[]);

IAP iap_entry;

void invoke_isp()
{
        iap_entry = (void*)IAP_LOCATION;
        unsigned int command[5] = {0};
        unsigned int output[5] = {0};

	printf("\n\nInvoking ISP...\n");

	LPC_SC->PLL0CON = 0;
	LPC_SC->PLL0CFG = 0;
	LPC_SC->CLKSRCSEL = LPC_SC_CLKSRCSEL_CLKSRC_RC;

	pll0_feed();

        command[0] = 57;
        iap_entry(command, output);
}
