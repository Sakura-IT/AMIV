#include "amiv_main.h"
#include "amiv_config.h"
#include "amiv_uart.h"
#include "amiv_i2c.h"
#include "amiv_util.h"
#include "amiv_flash.h"

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

			Word = AMIV_FLASH_read(Address);
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
			if(AMIV_FLASH_write(AddressActive, Val) == 0)
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
			AMIV_UART_SendString(" on flash ");
		}
		else
		{
			AMIV_UART_SendString("Syntax: c <12bit address>");
		}

		break;
	}
	case 'd': /* switch address in flash memory */
	{
		if(AMIV_FLASH_marker(0) == 0)
		{
			AMIV_UART_SendString("Default configuration is now used!\r\n");
		}
		break;
	}
	case 'f': /* flash configuration */
	{
		if(AMIV_FLASH_marker(1) == 0)
		{
			AMIV_UART_SendString("Custom configuration is now used!\r\n");
		}
		break;
	}
	default:
		AMIV_UART_SendString("Commands:\r\n");
		AMIV_UART_SendString("\t r: read register\r\n");
		AMIV_UART_SendString("\t w: write register\r\n");
		AMIV_UART_SendString("\t i: set active register (write to this reg)\r\n");
		AMIV_UART_SendString("\t s: set active device\r\n");
		AMIV_UART_SendString("\t z: read from flash\r\n");
		AMIV_UART_SendString("\t x: write to flash\r\n");
		AMIV_UART_SendString("\t c: set active address (write to this address, multiple of 4)\r\n");
		AMIV_UART_SendString("\t d: set to default configuration\r\n");
		AMIV_UART_SendString("\t f: set to custom configuration (from flash)\r\n");
	}
}
