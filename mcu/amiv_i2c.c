#include "stm32f0xx_i2c.h"
#include "amiv_i2c.h"
#include "amiv_main.h"
#include "amiv_config.h"

#define AMIV_I2C_SLAVE_ADDR_AD9984A	(0x98)
#define AMIV_I2C_SLAVE_ADDR_ADV7511	(0x72)
#define AMIV_I2C_SLAVE_ADDR_ADV7280	(0x40)
#define AMIV_I2C_SLAVE_ADDR_ADV7391	(0x56)
#define AMIV_I2C_SLAVE_ADDR_EDID	(0x7E)

char *SlaveString[5] =
{
	"AD9984A",
	"ADV7511",
	"ADV7280",
	"ADV7391",
	"EDID"
};

AMIV_I2C_Slave ActiveSlave = AMIV_I2C_SLAVE_AD9984A;

static uint8_t AMIV_I2C_GetSlaveAddress(AMIV_I2C_Slave S)
{
	switch(S)
	{
	case AMIV_I2C_SLAVE_AD9984A:
		return AMIV_I2C_SLAVE_ADDR_AD9984A;
	case AMIV_I2C_SLAVE_ADV7511:
		return AMIV_I2C_SLAVE_ADDR_ADV7511;
	case AMIV_I2C_SLAVE_EDID:
		return AMIV_I2C_SLAVE_ADDR_EDID;
	default:
		return AMIV_I2C_SLAVE_ADDR_AD9984A;
	}
}

void AMIV_I2C_Init()
{
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	/* Enable I2C1 Clock */
	RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

	/* Configure gpio settings */
	AMIV_CONFIG_I2C();

	/* Configure I2C1 settings */
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0x00;
	I2C_InitStructure.I2C_OwnAddress1 = 0x10; // MPU6050 7-bit adress = 0x68, 8-bit adress = 0xD0;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_Timing = 0x10320309; // 400KHz | 8MHz-0x00310309; 16MHz-0x10320309; 48MHz-0x50330309
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C1, &I2C_InitStructure);

	/* Enable I2C1 */
	I2C_Cmd(I2C1, ENABLE);
}

void AMIV_I2C_WR_Reg(uint8_t Reg, uint8_t Val)
{
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET);

	I2C_TransferHandling(I2C1, AMIV_I2C_GetSlaveAddress(ActiveSlave) + 0x1, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

	I2C_SendData(I2C1, Reg);

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TCR) == RESET);

	I2C_TransferHandling(I2C1, AMIV_I2C_GetSlaveAddress(ActiveSlave) + 0x1, 1, I2C_AutoEnd_Mode, I2C_No_StartStop);

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

	I2C_SendData(I2C1, Val);

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);

	I2C_ClearFlag(I2C1, I2C_FLAG_STOPF);
}

uint8_t AMIV_I2C_RD_Reg(uint8_t Reg)
{
	uint8_t SingleData = 0;

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET);

	I2C_TransferHandling(I2C1, AMIV_I2C_GetSlaveAddress(ActiveSlave), 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TXIS) == RESET);

	I2C_SendData(I2C1, Reg);

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_TC) == RESET);

	I2C_TransferHandling(I2C1, AMIV_I2C_GetSlaveAddress(ActiveSlave), 1, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET);

	SingleData = I2C_ReceiveData(I2C1);

	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF) == RESET);

	I2C_ClearFlag(I2C1, I2C_FLAG_STOPF);

	return SingleData;
}

void AMIV_I2C_RMW_Reg(uint8_t Reg, uint8_t BitsToClear, uint8_t BitsToSet)
{
	AMIV_I2C_WR_Reg(Reg, (AMIV_I2C_RD_Reg(Reg) & ~BitsToClear) | BitsToSet);
}

void AMIV_I2C_ChangeSlave(AMIV_I2C_Slave S)
{
	ActiveSlave = S;
}

char *AMIV_I2C_GetActiveSlave()
{
	return SlaveString[ActiveSlave];
}
