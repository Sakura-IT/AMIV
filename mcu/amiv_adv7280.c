#include "amiv_adv7280.h"



void AMIV_ADV7280_Init()
{

	/*
	 * :Free-run, 480i 60Hz YPrPb Out:
delay 10 ;
42 0F 00 ; Exit Power Down Mode [ADV7280 writes begin]
42 00 07 ; ADI Required Write
42 0C 37 ; Force Free run mode
42 02 54 ; Force standard to NTSC-M
42 14 11 ; Set Free-run pattern to 100% color bars
42 03 0C ; Enable Pixel & Sync output drivers
42 04 07 ; Power-up INTRQ, HS & VS pads
42 13 00 ; Enable INTRQ output driver
42 17 41 ; Enable SH1
42 1D 40 ; Enable LLC output driver
42 52 CD ; ADI Required Write
42 80 51 ; ADI Required Write
42 81 51 ; ADI Required Write
42 82 68 ; ADI Required Write [ADV7280 writes finished]
	 */


	AMIV_I2C_WR_Reg(0x0F, 0x00);
	AMIV_I2C_WR_Reg(0x00, 0x07);
	AMIV_I2C_WR_Reg(0x0C, 0x37);
	AMIV_I2C_WR_Reg(0x02, 0x54);
	AMIV_I2C_WR_Reg(0x14, 0x11);
	AMIV_I2C_WR_Reg(0x03, 0x0C);
	AMIV_I2C_WR_Reg(0x04, 0x07);
	AMIV_I2C_WR_Reg(0x13, 0x00);
	AMIV_I2C_WR_Reg(0x17, 0x41);
	AMIV_I2C_WR_Reg(0x1D, 0x40);
	AMIV_I2C_WR_Reg(0x52, 0xCD);
	AMIV_I2C_WR_Reg(0x80, 0x51);
	AMIV_I2C_WR_Reg(0x81, 0x51);
	AMIV_I2C_WR_Reg(0x82, 0x68);
}
