#ifndef __AMIV_UTIL_INC_GUARD
#define __AMIV_UTIL_INC_GUARD

#include <stdint.h>

#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include <stdlib.h>
#include <stdio.h>

char *AMIV_UTIL_itoa(int i);
int32_t AMIV_UTIL_atoi(char *p);
void AMIV_UTIL_memset(uint8_t *src, uint8_t value, uint32_t size);
char * AMIV_UTIL_itoahex(int i, int chars);
uint32_t AMIV_UTIL_strlen(char *p, char end);
int AMIV_UTIL_atox(char *p);

#endif
