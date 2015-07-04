#include "amiv_main.h"
#include "amiv_config.h"
#include "amiv_uart.h"
#include "amiv_i2c.h"
#include "amiv_util.h"

#define CHIP_ID 0x0

static uint8_t RegActive;

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
			Byte_p = AMIV_UTIL_itoahex(Byte);
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

			Reg_p = AMIV_UTIL_itoahex(RegActive);
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
	default:
		AMIV_UART_SendString("Commands:\r\n");
		AMIV_UART_SendString("\t r: read register\r\n");
		AMIV_UART_SendString("\t w: write register\r\n");
		AMIV_UART_SendString("\t i: set active register (write)\r\n");
		AMIV_UART_SendString("\t s: set active device\r\n");
	}
}
