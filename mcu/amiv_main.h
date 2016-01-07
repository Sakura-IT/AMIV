#ifndef __AMIV_MAIN_INC_GUARD
#define __AMIV_MAIN_INC_GUARD

#include <stdint.h>

#include "stm32f0xx_rcc.h"
#include <stdarg.h>

#define VERSION	"1.0"
#define NULL	(const int *)0
#define true	1
#define false	0

#define MAIN_DELAY							1000
#define MAIN_MEDIUM_DELAY					10000
#define MAIN_LONG_DELAY						100000
#define MAIN_STARTUP_DELAY					1000000
#define POLLING_INTERVAL					200000
#define BUTTON_LED_DELAY					40000
#define BUTTON_DEBOUNCE_LIMIT_LONG_PRESS	60000
#define BUTTON_LONG_PRESS					30000
#define BUTTON_MIN_VALID_PRESS				1000

typedef unsigned char bool;

#endif
