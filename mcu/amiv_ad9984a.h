#ifndef __AMIV_AD9984A_INC_GUARD_H
#define __AMIV_AD9984A_INC_GUARD_H

#include <stdint.h>

#define AD9984A_REG_INIT_1				0x1C
#define AD9984A_REG_INIT_2				0x20
#define AD9984A_REG_INIT_3				0x29
#define AD9984A_REG_INIT_4				0x2D
#define AD9984A_REG_INIT_5				0x2E

#define AD9984A_REG_POWER_1				0x03
#define AD9984A_REG_PLL_1				0x01
#define AD9984A_REG_PLL_2				0x02

typedef enum
{
	ad9984a_mode_amiga
} ad9984a_mode;

void AMIV_AD9984A_Init(ad9984a_mode mode);

#endif
