#include "stm32f0xx_gpio.h"
#include "amiv_main.h"
#include "amiv_config.h"
#include "amiv_uart.h"
#include "amiv_i2c.h"
#include "amiv_irq.h"
#include "amiv_adv7511.h"
#include "amiv_ad9984a.h"

#define DEBOUNCE_LIMIT_LONG_PRESS		60000
#define LONG_PRESS						30000
#define MIN_VALID_PRESS					5000
#define DELAY							1000

typedef enum
{

	BUTTON_STATE_INIT,
	BUTTON_STATE_IDLE,
	BUTTON_STATE_PRESS,
	BUTTON_STATE_RELEASE,
	BUTTON_STATE_LONG_PRESS,
}ButtonState_t;

typedef enum
{
	BUTTON_FUNC_SCROLL_RIGHT, /* 0 short press */
	BUTTON_FUNC_SCROLL_DOWN, /* 1 short press */
	BUTTON_FUNC_CHANGE_SIZE /* 2 short press */
}ButtonFunc_t;

int main(void)
{
	uint32_t i = 0;
	uint32_t ButtonCntPress = 0;
	ButtonState_t ButtonState = BUTTON_STATE_INIT;
	ButtonFunc_t ButtonFunc = BUTTON_FUNC_SCROLL_RIGHT;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	AMIV_CONFIG_GPIO();
	AMIV_CONFIG_LED();

	AMIV_CONFIG_Init();
	AMIV_UART_Init();
	AMIV_I2C_Init();
	AMIV_IRQ_Init();

	for(i = 0; i < 1000000; i++);

	AMIV_ADV7511_Init();
	AMIV_AD9984A_Init(ad9984a_mode_amiga);
	AMIV_ADV7511_PowerUp();

    while(1)
    {
    	/* GPIO pins are active low */

    	switch(ButtonState)
    	{
    	case BUTTON_STATE_INIT:
    		/* reset screen position */
    		GPIO_ResetBits(GPIOA, GPIO_Pin_15);
    		for(i = 0; i < DELAY; i++);
    		GPIO_SetBits(GPIOB, GPIO_Pin_4);
    		for(i = 0; i < DELAY; i++);
    		GPIO_ResetBits(GPIOB, GPIO_Pin_4);
    		for(i = 0; i < DELAY; i++);
    		GPIO_SetBits(GPIOB, GPIO_Pin_5);
    		for(i = 0; i < DELAY; i++);
    		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    		for(i = 0; i < DELAY; i++);
    		GPIO_SetBits(GPIOA, GPIO_Pin_15);
    		ButtonState = BUTTON_STATE_IDLE;
    		break;
    	case BUTTON_STATE_IDLE:
    		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
    		{
    			ButtonState = BUTTON_STATE_PRESS;
    		}
    		break;
    	case BUTTON_STATE_PRESS:
    		ButtonCntPress++;
    		if(ButtonCntPress >= LONG_PRESS)
    		{
    			ButtonState = BUTTON_STATE_LONG_PRESS;
    		}

    		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET)
    		{
    			ButtonState = BUTTON_STATE_RELEASE;
    		}
    		break;
    	case BUTTON_STATE_LONG_PRESS:
    		switch(ButtonFunc)
    		{
    		case BUTTON_FUNC_SCROLL_RIGHT:
    			GPIO_SetBits(GPIOB, GPIO_Pin_5);
    			for(i = 0; i < DEBOUNCE_LIMIT_LONG_PRESS; i++);
    			GPIO_ResetBits(GPIOB, GPIO_Pin_5);
    			for(i = 0; i < DEBOUNCE_LIMIT_LONG_PRESS; i++);

				if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET)
				{
					ButtonState = BUTTON_STATE_IDLE;
					ButtonCntPress = 0;
				}
				break;
    		case BUTTON_FUNC_SCROLL_DOWN:
				GPIO_SetBits(GPIOB, GPIO_Pin_4);
				for(i = 0; i < DEBOUNCE_LIMIT_LONG_PRESS; i++);
				GPIO_ResetBits(GPIOB, GPIO_Pin_4);
				for(i = 0; i < DEBOUNCE_LIMIT_LONG_PRESS; i++);

				if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET)
				{
					ButtonState = BUTTON_STATE_IDLE;
					ButtonCntPress = 0;
				}
    			break;
    		case BUTTON_FUNC_CHANGE_SIZE:
    			AMIV_ADV7511_DecreaseVerticalSize(0x20);
    			for(i = 0; i < DEBOUNCE_LIMIT_LONG_PRESS*15; i++);

				if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET)
				{
					ButtonState = BUTTON_STATE_IDLE;
					ButtonCntPress = 0;
				}
    			break;
    		}
    		break;
    	case BUTTON_STATE_RELEASE:
    		if(ButtonCntPress <= MIN_VALID_PRESS)
    		{
    			ButtonState = BUTTON_STATE_IDLE;
    		}
    		else
    		{
    			/* Change the function of the button */
    			if(ButtonFunc == BUTTON_FUNC_SCROLL_RIGHT)
    			{
    				ButtonFunc = BUTTON_FUNC_SCROLL_DOWN;
    			}
    			else if(ButtonFunc == BUTTON_FUNC_SCROLL_DOWN)
    			{
    				ButtonFunc = BUTTON_FUNC_CHANGE_SIZE;
    			}
    			else if(ButtonFunc == BUTTON_FUNC_CHANGE_SIZE)
    			{
    				ButtonFunc = BUTTON_FUNC_SCROLL_RIGHT;
    			}
    		}
    		ButtonCntPress = 0;
    		break;
    	}

#if 0
			/* Switch 2 */
			if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET)
			{
				/* Active ! */
				GPIO_SetBits(GPIOB, GPIO_Pin_4);
			}
			else
			{
				GPIO_ResetBits(GPIOB, GPIO_Pin_4);
			}

			/* Switch 3 */
			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == Bit_RESET)
			{
				/* Active ! */
				GPIO_SetBits(GPIOB, GPIO_Pin_3);
			}
			else
			{
				GPIO_ResetBits(GPIOB, GPIO_Pin_3);
			}

			/* Switch 4 */
			if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == Bit_RESET)
			{
				/* Active ! */
				GPIO_SetBits(GPIOA, GPIO_Pin_15);
			}
			else
			{
				GPIO_ResetBits(GPIOA, GPIO_Pin_15);
			}
#endif
    }
}
