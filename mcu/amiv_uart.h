#ifndef __AMIV_UART_INC_GUARD
#define __AMIV_UART_INC_GUARD

#include <stdint.h>

void AMIV_UART_Init();
void AMIV_UART_SendChar(char x);
void AMIV_UART_SendString(char *x);
ITStatus AMIV_UART_ResetStatus();
uint8_t AMIV_UART_ReadByte();

#endif
