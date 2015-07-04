#include "stm32f0xx_usart.h"
#include "amiv_main.h"
#include "amiv_uart.h"
#include "amiv_cmd.h"
#include "amiv_util.h"
#include "amiv_irq.h"
#include "amiv_config.h"

char AMIV_UART_Command[32];
uint8_t AMIV_UART_CommandCnt;

void AMIV_UART_Init()
{
  USART_InitTypeDef USART_InitStructure;

  /* Enable USART1 Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  /* Configure gpio settings */
  AMIV_CONFIG_UART();

  /* Enable USART1 IRQ */
  AMIV_IRQ_USART1_Enable();
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  /* Configure USART1 setting */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);

  /* Enable USART1 */
  USART_Cmd(USART1, ENABLE);

  AMIV_UTIL_memset((uint8_t *)AMIV_UART_Command, 0x00, 32);
  AMIV_UART_CommandCnt = 0;
}

void AMIV_UART_SendChar(char x)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, x);
}

void AMIV_UART_SendString(char *x)
{
	while(*x != '\0')
	{
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, *x);
		x++;
	}
}

ITStatus AMIV_UART_ResetStatus()
{
	return USART_GetITStatus(USART1, USART_IT_RXNE);
}

uint8_t AMIV_UART_ReadByte()
{
	return (uint8_t)USART_ReceiveData(USART1);
}
