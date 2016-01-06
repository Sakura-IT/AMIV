#include "amiv_main.h"
#include "amiv_config.h"
#include "amiv_uart.h"
#include "amiv_i2c.h"
#include "amiv_util.h"
#include "amiv_flash.h"
#include "amiv_button.h"
#include "amiv_adv7511.h"

#define CHIP_ID 0x0

static uint8_t RegActive;
static uint32_t AddressActive;

void AMIV_CMD_ExecuteCommand(char *CMD_p, uint8_t Count)
{
	switch(CMD_p[0])
	{
	case 'r': /* Read */
	{
		uint32_t Byte = 0;
		char *Byte_p;
		uint32_t Reg;

		if(Count >= 3)
		{
			/* if input is hex */
			if(CMD_p[3] == 'x')
			{
				Reg = AMIV_UTIL_atox(&CMD_p[4]);
			}
			else
			{
				Reg = AMIV_UTIL_atoi(&CMD_p[2]);
			}

			Byte = AMIV_I2C_RD_Reg(Reg);
			Byte_p = AMIV_UTIL_itoahex(Byte, 2);
			AMIV_UART_SendString("Read ");
			AMIV_UART_SendString(Byte_p);
			AMIV_UART_SendString(" on device ");
			AMIV_UART_SendString(AMIV_I2C_GetActiveSlave());
		}
		else
		{
			AMIV_UART_SendString("Syntax: r <reg>");
		}

		break;
	}
	case 'w': /* Write */
	{
		uint32_t Val;
		char *Reg_p;

		if(Count >= 3)
		{
			/* if input is hex */
			if(CMD_p[3] == 'x')
			{
				Val = AMIV_UTIL_atox(&CMD_p[4]);
			}
			else
			{
				Val = AMIV_UTIL_atoi(&CMD_p[2]);
			}

			Reg_p = AMIV_UTIL_itoahex(RegActive, 2);
			AMIV_I2C_WR_Reg(RegActive, Val);
			AMIV_UART_SendString("Written ");
			AMIV_UART_SendString(&CMD_p[2]);
			AMIV_UART_SendString(" to register ");
			AMIV_UART_SendString(Reg_p);
			AMIV_UART_SendString(" on device ");
			AMIV_UART_SendString(AMIV_I2C_GetActiveSlave());

		}
		else
		{
			AMIV_UART_SendString("Syntax: r <reg>");
		}

		break;
	}
	case 'i': /* switch register */
	{
		uint32_t Reg;

		if(Count >= 3)
		{
			/* if input is hex */
			if(CMD_p[3] == 'x')
			{
				Reg = AMIV_UTIL_atox(&CMD_p[4]);
			}
			else
			{
				Reg = AMIV_UTIL_atoi(&CMD_p[2]);
			}
			RegActive = Reg;
			AMIV_UART_SendString("Register active is ");
			AMIV_UART_SendString(&CMD_p[2]);
			AMIV_UART_SendString(" on device ");
			AMIV_UART_SendString(AMIV_I2C_GetActiveSlave());
		}
		else
		{
			AMIV_UART_SendString("Syntax: i <reg>");
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
			AMIV_UART_SendString("Syntax: s <slave>\r\n\t0: AD9984A\r\n\t1: ADV7511\r\n\t2: ADV7280\r\n\t3: ADV7391");
		}

		break;
	}
	case 'z': /* Flash read */
	{
		uint32_t Word = 0;
		char *Word_p;
		uint32_t Address;

		if(Count >= 3)
		{
			/* if input is hex */
			if(CMD_p[3] == 'x')
			{
				Address = AMIV_UTIL_atox(&CMD_p[4]);
			}
			else
			{
				Address = AMIV_UTIL_atoi(&CMD_p[2]);
			}

			Word = AMIV_FLASH_Read(Address);
			Word_p = AMIV_UTIL_itoahex(Word, 8);
			AMIV_UART_SendString("Read ");
			AMIV_UART_SendString(Word_p);
		}
		else
		{
			AMIV_UART_SendString("Syntax: z <12 bit address>");
		}

		break;
	}
	case 'x': /* Flash write */
	{
		uint32_t Val;
		char *Address_p;

		if(Count >= 3)
		{
			/* if input is hex */
			if(CMD_p[3] == 'x')
			{
				Val = AMIV_UTIL_atox(&CMD_p[4]);
			}
			else
			{
				Val = AMIV_UTIL_atoi(&CMD_p[2]);
			}

			Address_p = AMIV_UTIL_itoahex(AddressActive, 8);
			if(AMIV_FLASH_Write(AddressActive, Val) == 0)
			{
				AMIV_UART_SendString("Written ");
				AMIV_UART_SendString(&CMD_p[2]);
			}
			AMIV_UART_SendString(" at address ");
			AMIV_UART_SendString(Address_p);

		}
		else
		{
			AMIV_UART_SendString("Syntax: x <32bit value>");
		}

		break;
	}
	case 'c': /* switch address in flash memory */
	{
		uint32_t Address;

		if(Count >= 3)
		{
			/* if input is hex */
			if(CMD_p[3] == 'x')
			{
				Address = AMIV_UTIL_atox(&CMD_p[4]);
			}
			else
			{
				Address = AMIV_UTIL_atoi(&CMD_p[2]);
			}
			AddressActive = Address;
			AMIV_UART_SendString("Active address is ");
			AMIV_UART_SendString(&CMD_p[2]);
			AMIV_UART_SendString(" on flash");
		}
		else
		{
			AMIV_UART_SendString("Syntax: c <12bit address>");
		}

		break;
	}
	case 'd': /* default general configuration */
	{
		if(Count >= 3)
		{
			if(CMD_p[2] == 'Y' && CMD_p[3] == 'E' && CMD_p[4] == 'S')
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
			if(CMD_p[2] == 'Y' && CMD_p[3] == 'E' && CMD_p[4] == 'S')
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
		if(Count >= 3 && CMD_p[2] == 'Y' && CMD_p[3] == 'E' && CMD_p[4] == 'S')
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
			/* if input is hex */
			if(CMD_p[3] == 'x')
			{
				Page = AMIV_UTIL_atox(&CMD_p[4]);
			}
			else
			{
				Page = AMIV_UTIL_atoi(&CMD_p[2]);
			}

			if(AMIV_FLASH_Erase(Page) == 0)
			{
				AMIV_UART_SendString("Page ");
				AMIV_UART_SendString(&CMD_p[2]);
				AMIV_UART_SendString(" erased on flash ");
			}
		}
		else
		{
			AMIV_UART_SendString("Syntax: e <Page> where page is 1, 2, 3 or 4");
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
		NVIC_SystemReset();
	}
	case 'j':
	{
		uint8_t Offset;
		uint8_t Value;

		if(Count >= 7)
		{
			CMD_p[6] = '\0'; /* divide string */

			/* if input is hex */
			if(CMD_p[3] == 'x')
			{
				Offset = AMIV_UTIL_atox(&CMD_p[4]);
				if(CMD_p[8] == 'x')
				{
					Value = AMIV_UTIL_atox(&CMD_p[9]);
				}
				else
				{
					Value = AMIV_UTIL_atoi(&CMD_p[7]);
				}
			}
			else
			{
				AMIV_UART_SendString("Syntax: j <offset> <value> e.g. 'j 0x00 2'");
			}

			if(AMIV_FLASH_WriteButtonConfig(Offset, Value) == 0)
			{
				AMIV_UART_SendString("Done!");
			}
		}
		else
		{
			AMIV_UART_SendString("Syntax: j <offset> <value> e.g. 'j 0x00 2'");
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
	default:
		AMIV_UART_SendString("Commands:\r\n");
		AMIV_UART_SendString("\t r: read register\r\n");
		AMIV_UART_SendString("\t w: write register\r\n");
		AMIV_UART_SendString("\t i: set active register (will write to this reg)\r\n");
		AMIV_UART_SendString("\t s: set active device\r\n");
		AMIV_UART_SendString("\t z: read from flash\r\n");
		AMIV_UART_SendString("\t x: write to flash\r\n");
		AMIV_UART_SendString("\t c: set active address (will write to this address)\r\n");
		AMIV_UART_SendString("\t d: set to default general configuration (cannot be undone)\r\n");
		AMIV_UART_SendString("\t b: set to default button configuration (cannot be undone)\r\n");
		AMIV_UART_SendString("\t f: save current general config (in flash memory)\r\n");
		AMIV_UART_SendString("\t e: erase a page in flash\r\n");
		AMIV_UART_SendString("\t p: read EDID\r\n");
		AMIV_UART_SendString("\t q: Soft Reset\r\n");
		AMIV_UART_SendString("\t h: Instructions for programming a button\r\n");
		AMIV_UART_SendString("\t j: write button config in flash\r\n");
	}
}
