#include "amiv_main.h"
#include "amiv_config.h"
#include "amiv_uart.h"
#include "amiv_i2c.h"
#include "amiv_util.h"
#include "amiv_flash.h"
#include "amiv_button.h"
#include "amiv_adv7511.h"

#define CHIP_ID 0x0

static uint32_t GetValueFromString(char *String_p)
{
	/* if input is hex */
	if(String_p[1] == 'x')
	{
		return AMIV_UTIL_atox(&String_p[2]);
	}
	else
	{
		return AMIV_UTIL_atoi(String_p);
	}
}

/* This funciton will support one letter command together with two arguments */
void AMIV_CMD_ExecuteCommand(char *CMD_p, uint8_t Count)
{
	char *arg1_p;
	char *arg2_p;

	arg1_p = &CMD_p[2];
	arg2_p = &CMD_p[2];

	while(*arg2_p != ' ' && *arg2_p != '\0')
	{
		arg2_p++;
	}

	if(*arg2_p == ' ')
	{
		*arg2_p = '\0';
		arg2_p++;
	}

	switch(CMD_p[0])
	{
	case 'r': /* Read */
	{
		uint32_t Byte = 0;
		char *Byte_p;
		uint32_t Reg;

		if(Count >= 3)
		{
			Reg = GetValueFromString(arg1_p);

			Byte = AMIV_I2C_RD_Reg(Reg);
			Byte_p = AMIV_UTIL_itoahex(Byte, 2);
			AMIV_UART_SendString("Read ");
			AMIV_UART_SendString(Byte_p);
			AMIV_UART_SendString(" on device ");
			AMIV_UART_SendString(AMIV_I2C_GetActiveSlave());
		}
		else
		{
			AMIV_UART_SendString("Syntax: r <8bit reg>");
		}

		break;
	}
	case 'w': /* Write */
	{
		uint32_t Val;
		uint8_t Reg;

		if(Count >= 3)
		{
			Reg = GetValueFromString(arg1_p);
			Val = GetValueFromString(arg2_p);

			AMIV_I2C_WR_Reg(Reg, Val);
			AMIV_UART_SendString("Written ");
			AMIV_UART_SendString(arg2_p);
			AMIV_UART_SendString(" to register ");
			AMIV_UART_SendString(arg1_p);
			AMIV_UART_SendString(" on device ");
			AMIV_UART_SendString(AMIV_I2C_GetActiveSlave());

		}
		else
		{
			AMIV_UART_SendString("Syntax: w <8bit reg> <8bit value>");
		}

		break;
	}
	case 's': /* Change slave */
	{
		AMIV_I2C_Slave S;

		if(Count >= 3)
		{
			S = (AMIV_I2C_Slave)AMIV_UTIL_atoi(&CMD_p[2]);
			AMIV_I2C_ChangeSlave(S);
			AMIV_UART_SendString("Device ");
			AMIV_UART_SendString(AMIV_I2C_GetActiveSlave());
			AMIV_UART_SendString(" is active");
		}
		else
		{
			AMIV_UART_SendString("Syntax: s <slave> where slave is 0(AD9984A) or 1(ADV7511)\r\n");
		}

		break;
	}
	case 'z': /* Flash read */
	{
		uint32_t Word = 0;
		char *Word_p;
		uint32_t Offset;

		if(Count >= 3)
		{
			Offset = GetValueFromString(arg1_p);

			Word = AMIV_FLASH_Read(Offset);
			Word_p = AMIV_UTIL_itoahex(Word, 8);
			AMIV_UART_SendString("Read ");
			AMIV_UART_SendString(Word_p);
		}
		else
		{
			AMIV_UART_SendString("Syntax: z <10bit offset>");
		}

		break;
	}
	case 'x': /* Flash write */
	{
		uint32_t Val;
		uint32_t Offset;

		if(Count >= 3)
		{
			Val = GetValueFromString(arg1_p);
			Offset = GetValueFromString(arg2_p);

			if(AMIV_FLASH_Write(Offset, Val) == 0)
			{
				AMIV_UART_SendString("Written ");
				AMIV_UART_SendString(arg1_p);
			}
			AMIV_UART_SendString(" at address ");
			AMIV_UART_SendString(arg2_p);

		}
		else
		{
			AMIV_UART_SendString("Syntax: x <10bit offset> <32bit value>");
		}

		break;
	}
	case 'd': /* default general configuration */
	{
		if(Count >= 3)
		{
			if(arg1_p[0] == 'Y' && arg1_p[1] == 'E' && arg1_p[2] == 'S')
			{
				if(AMIV_FLASH_Erase(PAGE_GENERAL_CONFIG) == 0)
				{
					AMIV_UART_SendString("Default general configuration is now used!");
				}
			}
		}
		else
		{
			AMIV_UART_SendString("Syntax: d YES");
		}
		break;
	}
	case 'b': /* default button configuration */
	{
		if(Count >= 3)
		{
			if(arg1_p[0] == 'Y' && arg1_p[1] == 'E' && arg1_p[2] == 'S')
			{
				if(AMIV_FLASH_Erase(PAGE_BUTTION_CONFIG) == 0)
				{
					AMIV_UART_SendString("Default button configuration is now used!");
				}
			}
		}
		else
		{
			AMIV_UART_SendString("Syntax: b YES");
		}
		break;
	}
	case 'f': /* Save general config */
	{
		if(arg1_p[0] == 'Y' && arg1_p[1] == 'E' && arg1_p[2] == 'S')
		{
			if(AMIV_BUTTON_SaveGeneralConfig() == 0)
			{
				AMIV_UART_SendString("General configuration has been saved on flash!");
			}
			else
			{
				AMIV_UART_SendString("Could not save general configuration to flash memory!");
			}
		}
		else
		{
			AMIV_UART_SendString("Syntax: f YES");
		}
		break;
	}
	case 'e': /* switch address in flash memory */
	{
		uint8_t Page;

		if(Count >= 3)
		{
			Page = GetValueFromString(arg1_p);

			if(AMIV_FLASH_Erase(Page) == 0)
			{
				AMIV_UART_SendString("Page ");
				AMIV_UART_SendString(&CMD_p[2]);
				AMIV_UART_SendString(" erased on flash ");
			}
		}
		else
		{
			AMIV_UART_SendString("Syntax: e <2bit Page>");
		}

		break;
	}
	case 'p':
	{
		char *Reg_p;
		uint16_t i;
		uint8_t *EDID_p;

		AMIV_ADV7511_ReadEDID();

		EDID_p = AMIV_ADV7511_GetEDIDPointer();

		for(i = 0; i < 128; i++)
		{
			Reg_p = AMIV_UTIL_itoahex((int)*(EDID_p + i), 2);
			AMIV_UART_SendString(Reg_p);
			AMIV_UART_SendString(" ");
			if(!((i + 1) % 16))
			{
				AMIV_UART_SendString("\r\n");
			}
		}
		break;
	}
	case 'q':
	{
		AMIV_UART_SendString("Resetting...\r\n\r\n");
		NVIC_SystemReset();
	}
	case 'j':
	{
		uint8_t Offset;
		uint8_t Value;

		if(Count >= 3)
		{
			Offset = GetValueFromString(arg1_p);
			Value = GetValueFromString(arg2_p);

			if(AMIV_FLASH_WriteButtonConfig(Offset, Value) == 0)
			{
				AMIV_UART_SendString("Done!");
			}
		}
		else
		{
			AMIV_UART_SendString("Syntax: j <offset> <attribute> use 'h' cmd for instructions");
		}

		break;
	}
	case 'y':
	{
		uint8_t Button;

		if(Count >= 3)
		{
			Button = GetValueFromString(arg1_p);

			if(Button < 1 || Button > 7)
			{
				AMIV_UART_SendString("Syntax: e <Button> where button is 1-7");
			}
			else
			{
				AMIV_BUTTON_RemoveButtonConfiguration(Button - 1);
				AMIV_UART_SendString("Done!");
			}
		}
		else
		{
			AMIV_UART_SendString("Syntax: y <Button> where button is 1-7");
		}

		break;
	}
	case 'h':
	{
		AMIV_UART_SendString("Format for button configuration\r\n\r\n");
		AMIV_UART_SendString("Button 1:\r\n");
		AMIV_UART_SendString("0x00 ButtonAction [BUTTON_ACTION_FPGA = 0, BUTTON_ACTION_REG = 1, BUTTON_ACTION_SAVE = 2, BUTTON_ACTION_RESET = 3]\r\n");
		AMIV_UART_SendString("0x08 ShortPress [set to 1 if action should be performed upon short press]\r\n");
		AMIV_UART_SendString("0x09 LongPress [set to 1 if action should be performed upon long press]\r\n");
		AMIV_UART_SendString("0x0A ContinueLongPress [set to 1 if action should be repeated upon long press]\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("BUTTON_ACTION_FPGA\r\n");
		AMIV_UART_SendString("0x01 FPGAAction [FPGA_ACTION_SCROLL_RIGHT = 0, FPGA_ACTION_SCROLL_LEFT = 1, FPGA_ACTION_SCROLL_UP = 2, FPGA_ACTION_SCROLL_DOWN = 3]\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("BUTTON_ACTION_REG\r\n");
		AMIV_UART_SendString("0x01 Chip [AD9984A = 0, ADV7511 = 1]\r\n");
		AMIV_UART_SendString("0x02 UsingMSBReg [if MSB register is used, then set to 1, otherwise set to 0]\r\n");
		AMIV_UART_SendString("0x03 RegMSB [MSB register, e.g. 0x21]\r\n");
		AMIV_UART_SendString("0x04 RegLSB [LSB register, e.g. 0x21]\r\n");
		AMIV_UART_SendString("0x05 Sign [should the register value be increased with 'Step' set to 1, otherwise set to 0]\r\n");
		AMIV_UART_SendString("0x06 Step [the quantity that should be added or subtracted to the register value]\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("No extra information is needed when action is set to BUTTON_ACTION_SAVE or BUTTON_ACTION_RESET\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("Button 2:\r\n");
		AMIV_UART_SendString("Add 0x10 offset to flash address\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("To program button 5 to modify the input clock for AD9984A chip:\r\n");
		AMIV_UART_SendString("(address must be written as 4 characters)\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("j 0x40 1\r\n");
		AMIV_UART_SendString("j 0x48 1\r\n");
		AMIV_UART_SendString("j 0x49 1\r\n");
		AMIV_UART_SendString("j 0x4A 1\r\n");
		AMIV_UART_SendString("j 0x41 0\r\n");
		AMIV_UART_SendString("j 0x42 1\r\n");
		AMIV_UART_SendString("j 0x43 0x01\r\n");
		AMIV_UART_SendString("j 0x44 0x02\r\n");
		AMIV_UART_SendString("j 0x45 1\r\n");
		AMIV_UART_SendString("j 0x46 0x10\r\n");
		AMIV_UART_SendString("q\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("When pressing button 4 the clock will be increased with 0x10\r\n");
		break;
	}
	case 'v':
	{
		AMIV_UART_SendString(VERSION);
		break;
	}
	default:
		AMIV_UART_SendString("I2C\r\n");
		AMIV_UART_SendString("\t r: read register\r\n");
		AMIV_UART_SendString("\t w: write register\r\n");
		AMIV_UART_SendString("\t s: set active device\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("FLASH\r\n");
		AMIV_UART_SendString("\t z: read 32bit from flash\r\n");
		AMIV_UART_SendString("\t x: write 32bit to flash\r\n");
		AMIV_UART_SendString("\t e: erase a page in flash\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("GENERAL CONFIG\r\n");
		AMIV_UART_SendString("\t f: save current general config (in flash memory)\r\n");
		AMIV_UART_SendString("\t d: default general configuration (cannot be undone)\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("BUTTON CONFIG\r\n");
		AMIV_UART_SendString("\t j: write button config in flash\r\n");
		AMIV_UART_SendString("\t b: default button configuration (cannot be undone)\r\n");
		AMIV_UART_SendString("\t y: remove one button configuration (cannot be undone)\r\n");
		AMIV_UART_SendString("\t h: Instructions for programming button\r\n");
		AMIV_UART_SendString("\r\n");
		AMIV_UART_SendString("MISC\r\n");
		AMIV_UART_SendString("\t p: read EDID\r\n");
		AMIV_UART_SendString("\t q: soft reset\r\n");
		AMIV_UART_SendString("\t v: print version\r\n");
	}
}
