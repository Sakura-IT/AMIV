#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_syscfg.h"
#include "amiv_irq.h"
#include "amiv_main.h"
#include "amiv_uart.h"
#include "amiv_adv7511.h"
#include "amiv_ad9984a.h"
#include "amiv_i2c.h"
#include "amiv_util.h"
#include "amiv_cmd.h"

extern char AMIV_UART_Command[32];
extern uint8_t AMIV_UART_CommandCnt;
extern char *AMIV_ADV7511_ErrorStrings[];

void EXTI4_15_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		uint8_t Val;
		uint32_t i;
		char *Byte_p;

		AMIV_UART_SendString("IRQ RECIVED LINE 5!\r\n");

		AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_ADV7511);
		for(i = 0; i < 10000;i++)
		{
			;
		}
		Val = AMIV_I2C_RD_Reg(AMIV_ADV7511_REG_IRQ);
		Byte_p = AMIV_UTIL_itoahex(Val);
		AMIV_UART_SendString("IRQ status: ");
		AMIV_UART_SendString(Byte_p);
		AMIV_UART_SendString("\r\n");

		/* is this interrupt about HPD? */
		if(Val & 0x80)
		{
			/* is HPD asserted ? */
			if(AMIV_I2C_RD_Reg(AMIV_ADV7511_REG_DETECT) & 0x40)
			{
				AMIV_UART_SendString("Cable inserted, initializing...\r\n");
				AMIV_ADV7511_PowerUp();
			}
			else
			{
				AMIV_UART_SendString("Cable removed\r\n");
			}
		}
		else if(AMIV_I2C_RD_Reg(AMIV_ADV7511_REG_STATUS) & 0xF0)
		{
			AMIV_UART_SendString(AMIV_ADV7511_ErrorStrings[AMIV_I2C_RD_Reg(AMIV_ADV7511_REG_STATUS) >> 4]);
		}
		else if(Val & 0x04)
		{
			char *Reg_p;
			uint16_t i;
			uint8_t *EDID_p;

			AMIV_UART_SendString("Reading EDID:\r\n");
			//AMIV_ADV7511_ReadEDID();

			/*EDID_p = AMIV_ADV7511_GetEDIDPointer();

			for(i = 0; i < 128; i++)
			{
				Reg_p = AMIV_UTIL_itoahex((int)*(EDID_p + i));
				AMIV_UART_SendString(Reg_p);
				AMIV_UART_SendString(" ");
				if(!((i + 1) % 16))
				{
					AMIV_UART_SendString("\r\n");
				}
			}*/
		}

		AMIV_I2C_WR_Reg(AMIV_ADV7511_REG_IRQ, Val);

		EXTI_ClearITPendingBit(EXTI_Line5);
	}
	else if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		AMIV_UART_SendString("IRQ RECIVED LINE 8!\r\n");

		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	else if(EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		AMIV_UART_SendString("IRQ RECIVED LINE 4!\r\n");

		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}

void USART1_IRQHandler(void)
{
	if(AMIV_UART_ResetStatus() != RESET)
	{
		AMIV_UART_Command[AMIV_UART_CommandCnt] = AMIV_UART_ReadByte();
		AMIV_UART_SendChar(AMIV_UART_Command[AMIV_UART_CommandCnt]); /* Remote Echo */
		if(AMIV_UART_Command[AMIV_UART_CommandCnt] == '\r') /* If CR */
		{
			AMIV_UART_Command[AMIV_UART_CommandCnt] = '\0';
			AMIV_UART_SendChar('\n');
			AMIV_CMD_ExecuteCommand(AMIV_UART_Command, AMIV_UART_CommandCnt);
			AMIV_UART_SendChar('\r');
			AMIV_UART_SendChar('\n');
			AMIV_UART_SendChar('\n');

			AMIV_UTIL_memset((uint8_t *)AMIV_UART_Command, 0x00, 32);
			AMIV_UART_CommandCnt = 0;
		}
		else
		{
			AMIV_UART_CommandCnt++;
		}
	}
}

void AMIV_IRQ_Init()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* ADV7511 IRQ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* ADV7280 IRQ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* DEBUG IRQ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4);

	EXTI_InitStructure.EXTI_Line = EXTI_Line5 | EXTI_Line8 | EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void AMIV_IRQ_USART1_Enable()
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
