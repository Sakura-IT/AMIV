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
#define AD9984A_REG_SYNC				0x24

typedef enum
{
	ad9984a_mode_amiga
} ad9984a_mode;

void AMIV_AD9984A_Init(ad9984a_mode mode);
uint8_t AMIV_AD9984A_HSYNCActive();

static const uint8_t AD9984A_WRRegList[] = {
              0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23,                         0x28, 0x29,             0x2C, 0x2D, 0x2E,
                                0x34,       0x36,                               0x3C,
};

#define AD9984A_WR_REG_LIST_COUNT		43

#endif
