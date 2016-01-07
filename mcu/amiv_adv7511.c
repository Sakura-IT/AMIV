#include "amiv_adv7511.h"
#include "amiv_main.h"
#include "amiv_i2c.h"

#define MIN_SIZE 		0x0AD0
#define MAX_SIZE 		0x0300
#define STEP_SIZE 		0x20

uint8_t AMIV_ADV7511_EDID[256 + 1];
uint16_t VerticalSize = 0x0000;

char *AMIV_ADV7511_ErrorStrings[] =
{
	"Bad Receiver BKSV",
	"Ri Mismatch",
	"Pj Mismatch",
	"I2C Error (usually a no-ack)",
	"Timed Out Waiting for Downstream Repeater DONE",
	"Max Cascade of Repeaters Exceeded",
	"SHA-1 Hash Check of KSV List Failed",
	"Too Many Devices Connected to Repeater Tree"
};

void AMIV_ADV7511_Init()
{
	/* switch to ADV7511 */
	AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_ADV7511);

	AMIV_I2C_WR_Reg(0x96, 0xFF);
}


void AMIV_ADV7511_PowerUp()
{
	/* switch to ADV7511 */
	AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_ADV7511);

	/* common stuff */
	AMIV_I2C_WR_Reg(0x01, 0x00);
	AMIV_I2C_WR_Reg(0x02, 0x18);
	AMIV_I2C_WR_Reg(0x03, 0x00);
	AMIV_I2C_WR_Reg(0x15, 0x00);
	AMIV_I2C_WR_Reg(0x16, 0x00);
	AMIV_I2C_WR_Reg(0x18, 0x08);

	/* de generation stuff */
	AMIV_I2C_WR_Reg(0x30, 0x04);
	AMIV_I2C_WR_Reg(0x31, 0x03);
	AMIV_I2C_WR_Reg(0x32, 0xE0);
	AMIV_I2C_WR_Reg(0x33, 0x24);
	AMIV_I2C_WR_Reg(0x34, 0x06);
	AMIV_I2C_WR_Reg(0x35, 0x40);
	AMIV_I2C_WR_Reg(0x36, 0xD9);
	AMIV_I2C_WR_Reg(0x37, 0x0A); /* output width */
	AMIV_I2C_WR_Reg(0x38, 0x00); /* output width */
	AMIV_I2C_WR_Reg(0x39, 0x2D);
    AMIV_I2C_WR_Reg(0x3A, 0x00);
	AMIV_I2C_WR_Reg(0x3B, 0x40);
	AMIV_I2C_WR_Reg(0x3C, 0x04);

	/* common stuff */
	AMIV_I2C_WR_Reg(0x40, 0x80);
	AMIV_I2C_WR_Reg(0x41, 0x10);
	AMIV_I2C_WR_Reg(0x49, 0xA8);
	AMIV_I2C_WR_Reg(0x55, 0x00);
	AMIV_I2C_WR_Reg(0x56, 0x08);
	AMIV_I2C_WR_Reg(0x96, 0x20);
	AMIV_I2C_WR_Reg(0x98, 0x03);
	AMIV_I2C_WR_Reg(0x99, 0x02);
	AMIV_I2C_WR_Reg(0x9C, 0x30);
	AMIV_I2C_WR_Reg(0x9D, 0x61);
	AMIV_I2C_WR_Reg(0xA2, 0xA4);
	AMIV_I2C_WR_Reg(0xA3, 0xA4);
	AMIV_I2C_WR_Reg(0xA5, 0x44);
	AMIV_I2C_WR_Reg(0xAB, 0x40);
	AMIV_I2C_WR_Reg(0xAF, 0x14);
	AMIV_I2C_WR_Reg(0xBA, 0xA0);
	AMIV_I2C_WR_Reg(0xDE, 0x9C);
	AMIV_I2C_WR_Reg(0xE4, 0x60);
	AMIV_I2C_WR_Reg(0xFA, 0x7D);
	AMIV_I2C_WR_Reg(0x48, 0x00);
	AMIV_I2C_WR_Reg(0x55, 0x10);
	AMIV_I2C_WR_Reg(0x56, 0x00);

	AMIV_I2C_RMW_Reg(0x16, 0x30, 0x30); /* Color Depth = 8 bit */
	AMIV_I2C_RMW_Reg(0x17, 0x00, 0x01);  /* DE Generator Enable = Enable */
}

void AMIV_ADV7511_ReadEDID()
{
	uint16_t i;

	AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_EDID);

	for(i = 0; i < 128; i++)
	{
		AMIV_ADV7511_EDID[i] = AMIV_I2C_RD_Reg(i);
		for(i = 0; i < MAIN_MEDIUM_DELAY;i++);
	}
}

uint8_t *AMIV_ADV7511_GetEDIDPointer()
{
	return AMIV_ADV7511_EDID;
}

