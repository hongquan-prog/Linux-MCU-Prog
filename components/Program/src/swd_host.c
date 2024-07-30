#include "DAP_config.h"
#include "DAP.h"

uint8_t swd_init(void)
{
	DAP_Setup();

	return 1;
}

uint8_t swd_off(void)
{
	PORT_OFF();
	return 1;
}

void swd_set_target_reset(uint8_t asserted)
{
	(asserted) ? PIN_nRESET_OUT(0) : PIN_nRESET_OUT(1);
}

