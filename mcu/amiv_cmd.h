#ifndef __AMIV_CMD_INC_GUARD_H
#define __AMIV_CMD_INC_GUARD_H

#include <stdint.h>

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"

void AMIV_CMD_ExecuteCommand(char *CMD_p, uint8_t Count);

#endif
