#ifndef __AMIV_I2C_INC_GUARD
#define __AMIV_I2C_INC_GUARD

#include <stdint.h>

typedef enum
{
	AMIV_I2C_SLAVE_AD9984A,
	AMIV_I2C_SLAVE_ADV7511,
	AMIV_I2C_SLAVE_EDID
} AMIV_I2C_Slave;

void AMIV_I2C_Init();
void AMIV_I2C_WR_Reg(uint8_t Reg, uint8_t Val);
uint8_t AMIV_I2C_RD_Reg(uint8_t Reg);
void AMIV_I2C_RMW_Reg(uint8_t Reg, uint8_t BitsToClear, uint8_t BitsToSet);
void AMIV_I2C_ChangeSlave(AMIV_I2C_Slave S);
char *AMIV_I2C_GetActiveSlave();

#endif
