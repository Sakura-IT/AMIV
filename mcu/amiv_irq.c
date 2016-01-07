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
#include "amiv_fpga.h"

extern char AMIV_UART_Command[32];
extern uint8_t AMIV_UART_CommandCnt;
extern char *AMIV_ADV7511_ErrorStrings[];

void EXTI4_15_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		uint8_t Val;
		char *Byte_p;

		AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_ADV7511);
		Val = AMIV_I2C_RD_Reg(AMIV_ADV7511_REG_IRQ);
		Byte_p = AMIV_UTIL_itoahex(Val, 2);
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
				AMIV_UART_SendString("Cable removed!\r\n");
			}
		}
		/* is this interrupt about monitor sense? */
		if(Val & 0x40)
		{
			AMIV_UART_SendString("Monitor sense detected\r\n");
		}

		if(Val & 0x20)
		{
			AMIV_UART_SendString("Vsync detected\r\n");
		}

		if(Val & 0x04)
		{
			AMIV_UART_SendString("EDID is available\r\n");
		}

		if(AMIV_I2C_RD_Reg(AMIV_ADV7511_REG_STATUS) & 0xF0)
		{
			AMIV_UART_SendString(AMIV_ADV7511_ErrorStrings[AMIV_I2C_RD_Reg(AMIV_ADV7511_REG_STATUS) >> 4]);
		}

		AMIV_I2C_WR_Reg(AMIV_ADV7511_REG_IRQ, Val);

		EXTI_ClearITPendingBit(EXTI_Line5);
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

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource5);

	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
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
