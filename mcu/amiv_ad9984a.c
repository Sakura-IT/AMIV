#include "amiv_ad9984a.h"
#include "amiv_main.h"
#include "amiv_i2c.h"

void AMIV_AD9984A_Init(ad9984a_mode mode)
{
	AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_AD9984A);

	switch(mode)
	{
	case ad9984a_mode_amiga:
		/* 576i */
		AMIV_I2C_WR_Reg(0x0, 0x01);
		AMIV_I2C_WR_Reg(0x1, 0x38); /* sample clock */
		AMIV_I2C_WR_Reg(0x2, 0xC0); /* sample clock */
		AMIV_I2C_WR_Reg(0x3, 0x18);
		AMIV_I2C_WR_Reg(0x4, 0x78); /* ADC fine adjustment */

		/* contrast */
		AMIV_I2C_WR_Reg(0x5, 0x00);
		AMIV_I2C_WR_Reg(0x6, 0x80);
		AMIV_I2C_WR_Reg(0x7, 0x00);
		AMIV_I2C_WR_Reg(0x8, 0x80);
		AMIV_I2C_WR_Reg(0x9, 0x00);
		AMIV_I2C_WR_Reg(0xA, 0x80);

		/* brightness */
		AMIV_I2C_WR_Reg(0xB, 0x00);
		AMIV_I2C_WR_Reg(0xC, 0x80);
		AMIV_I2C_WR_Reg(0xD, 0x00);
		AMIV_I2C_WR_Reg(0xE, 0x80);
		AMIV_I2C_WR_Reg(0xF, 0x00);
		AMIV_I2C_WR_Reg(0x10, 0x80);

		/* general stuff */
		AMIV_I2C_WR_Reg(0x11, 0x20);
		AMIV_I2C_WR_Reg(0x12, 0x00);
		AMIV_I2C_WR_Reg(0x13, 0x20);
		AMIV_I2C_WR_Reg(0x14, 0x04);
		AMIV_I2C_WR_Reg(0x15, 0x0A);
		AMIV_I2C_WR_Reg(0x16, 0x06);
		AMIV_I2C_WR_Reg(0x17, 0x06);
		AMIV_I2C_WR_Reg(0x18, 0x00);
		AMIV_I2C_WR_Reg(0x19, 0x08);
		AMIV_I2C_WR_Reg(0x1A, 0x00);

		/* auto offset */
		AMIV_I2C_WR_Reg(0x1B, 0xFF);

		/* boring stuff */
		AMIV_I2C_WR_Reg(0x1C, 0xFF);
		AMIV_I2C_WR_Reg(0x1D, 0x78);
		AMIV_I2C_WR_Reg(0x1E, 0x34);

		/* output mode */
		AMIV_I2C_WR_Reg(0x1F, 0x90);

		/* pixel clock etc. */
		AMIV_I2C_WR_Reg(0x20, 0x07);

		/* boring stuff */
		AMIV_I2C_WR_Reg(0x21, 0x20);
		AMIV_I2C_WR_Reg(0x22, 0x32);
		AMIV_I2C_WR_Reg(0x23, 0x14);
		AMIV_I2C_WR_Reg(0x24, 0x08);
		AMIV_I2C_WR_Reg(0x25, 0x7F);
		AMIV_I2C_WR_Reg(0x26, 0x10);
		AMIV_I2C_WR_Reg(0x27, 0x70);
		AMIV_I2C_WR_Reg(0x28, 0xBF);
		AMIV_I2C_WR_Reg(0x29, 0x02);
		AMIV_I2C_WR_Reg(0x2A, 0x00);
		AMIV_I2C_WR_Reg(0x2B, 0x00);
		AMIV_I2C_WR_Reg(0x2C, 0x00);
		AMIV_I2C_WR_Reg(0x2D, 0x08);
		AMIV_I2C_WR_Reg(0x2E, 0x20);
		AMIV_I2C_WR_Reg(0x36, 0x01);

		/* auto gain matching */
		AMIV_I2C_WR_Reg(0x3C, 0x0E);
		break;
	default:
		break;

	}
}

uint8_t AMIV_AD9984A_HSYNCActive()
{
	AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_AD9984A);
	return AMIV_I2C_RD_Reg(AD9984A_REG_SYNC) & 0x80;
}
