#include "stm32f0xx_gpio.h"
#include "amiv_main.h"
#include "amiv_button.h"
#include "amiv_adv7511.h"
#include "amiv_ad9984a.h"
#include "amiv_i2c.h"
#include "amiv_flash.h"
#include "amiv_uart.h"
#include "amiv_fpga.h"

#define DEFAULT_CHIP					0x01
#define DEFAULT_REG_MSB					0x37
#define DEFAULT_REG_LSB					0x38
#define DEFAULT_STEP					0x08

typedef enum
{
	BUTTON_STATE_INIT,
	BUTTON_STATE_IDLE,
	BUTTON_STATE_PRESS,
	BUTTON_STATE_RELEASE,
	BUTTON_STATE_LONG_PRESS
}ButtonState_t;

ButtonState_t gButtonState;
Button_t gButton;
uint32_t gButtonCntPress;
uint32_t gArray_p[AD9984A_WR_REG_LIST_COUNT + ADV7511_WR_REG_LIST_COUNT];
ButtonConfig_t gButtonConfig[NUMBER_OF_BUTTONS];

static uint8_t SaveGeneralConfig()
{
	//uint32_t *Array_p;
	uint32_t i = 0;
	uint32_t ArrayCounter = 0;
	uint8_t Chip;
	uint8_t Reg;
	uint8_t Val;
	uint32_t Concat = 0;
	uint8_t Status = 0;

	/* switch to AD9984A */
	AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_AD9984A);
	Chip = (uint8_t)AMIV_I2C_SLAVE_AD9984A;

	for(i = 0; i < AD9984A_WR_REG_LIST_COUNT; i++)
	{
		Concat = 0;
		Reg = AD9984A_WRRegList[i];
		Val = AMIV_I2C_RD_Reg(Reg);

		Concat |= (Chip << 24);
		Concat |= (Reg << 16);
		Concat |= Val;
		gArray_p[ArrayCounter] = Concat;

		ArrayCounter++;
	}

	/* switch to ADV7511 */
	AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_ADV7511);
	Chip = (uint8_t)AMIV_I2C_SLAVE_ADV7511;

	for(i = 0; i < ADV7511_WR_REG_LIST_COUNT; i++)
	{
		Concat = 0;
		Reg = ADV7511_WRRegList[i];
		Val = AMIV_I2C_RD_Reg(Reg);

		Concat |= (Chip << 24);
		Concat |= (Reg << 16);
		Concat |= Val;
		gArray_p[ArrayCounter] = Concat;

		ArrayCounter++;
	}

	Status = AMIV_FLASH_WriteGeneralConfig(gArray_p, ArrayCounter);

	return Status;
}

static void Scroll(FPGAAction_t Action)
{
	switch(Action)
	{
	case FPGA_ACTION_SCROLL_RIGHT:
		AMIV_FPGA_ScrollRight();
		break;
	case FPGA_ACTION_SCROLL_LEFT:
		AMIV_FPGA_ScrollLeft();
		break;
	case FPGA_ACTION_SCROLL_UP:
		AMIV_FPGA_ScrollUp();
		break;
	case FPGA_ACTION_SCROLL_DOWN:
		AMIV_FPGA_ScrollDown();
		break;
	}
}

static void ExeciteButtonFunction()
{
	switch(gButtonConfig[gButton].ButtonAction)
	{
	case BUTTON_ACTION_FPGA:
		Scroll(gButtonConfig[gButton].FPGAFunction.FPGAAction);
		break;
	case BUTTON_ACTION_REG:
		{
			uint16_t Val = 0;

			AMIV_I2C_ChangeSlave(gButtonConfig[gButton].RegFunction.Chip);

			if(gButtonConfig[gButton].RegFunction.UsingMSBReg)
			{
				Val |= AMIV_I2C_RD_Reg(gButtonConfig[gButton].RegFunction.RegMSB) << 8;
			}
			Val |= AMIV_I2C_RD_Reg(gButtonConfig[gButton].RegFunction.RegLSB);
			if(gButtonConfig[gButton].RegFunction.Sign)
			{
				Val += gButtonConfig[gButton].RegFunction.Step;
			}
			else
			{
				Val -= gButtonConfig[gButton].RegFunction.Step;
			}

			if(gButtonConfig[gButton].RegFunction.UsingMSBReg)
			{
				AMIV_I2C_WR_Reg(gButtonConfig[gButton].RegFunction.RegMSB, (Val >> 8) & 0xFF);
			}
			AMIV_I2C_WR_Reg(gButtonConfig[gButton].RegFunction.RegLSB, Val & 0xFF);
		}
		break;
	case BUTTON_ACTION_SAVE:
		if(AMIV_BUTTON_SaveGeneralConfig() == 0)
		{
			AMIV_UART_SendString("General configuration has been saved on flash!\r\n");
		}
		else
		{
			AMIV_UART_SendString("Could not save general configuration to flash memory!\r\n");
		}
		break;
	case BUTTON_ACTION_RESET:
		NVIC_SystemReset();
		break;
	}

	/* Toggle led */
	if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_8))
	{
		uint32_t i;
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);
		for(i = 0; i < BUTTON_LED_DELAY; i++);
		GPIO_SetBits(GPIOA, GPIO_Pin_8);
	}
	else
	{
		uint32_t i;
		GPIO_SetBits(GPIOA, GPIO_Pin_8);
		for(i = 0; i < BUTTON_LED_DELAY; i++);
		GPIO_ResetBits(GPIOA, GPIO_Pin_8);
	}
}

static void ExecuteButtonFunctionShortPress()
{
	if(gButtonConfig[gButton].ShortPress)
	{
		ExeciteButtonFunction();
	}
}

static void ExecuteButtonFunctionLongPress()
{
	if(gButtonConfig[gButton].LongPress)
	{
		ExeciteButtonFunction();
	}
}

static Button_t Scan()
{
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET)
	{
		return BUTTON_1;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == Bit_RESET)
	{
		return BUTTON_2;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == Bit_RESET)
	{
		return BUTTON_3;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == Bit_RESET)
	{
		return BUTTON_4;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == Bit_RESET)
	{
		return BUTTON_5;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == Bit_RESET)
	{
		return BUTTON_6;
	}
	else if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_RESET)
	{
		return BUTTON_7;
	}

	return BUTTON_NONE;
}

void AMIV_BUTTON_Init()
{
	gButtonState = BUTTON_STATE_INIT;
	gButtonCntPress = 0;

	gButtonConfig[0].ButtonAction = BUTTON_ACTION_FPGA;
	gButtonConfig[0].FPGAFunction.FPGAAction = FPGA_ACTION_SCROLL_RIGHT;
	gButtonConfig[0].LongPress = 1;
	gButtonConfig[0].ShortPress = 1;
	gButtonConfig[0].ContinueLongPress = 1;

	gButtonConfig[1].ButtonAction = BUTTON_ACTION_FPGA;
	gButtonConfig[1].FPGAFunction.FPGAAction = FPGA_ACTION_SCROLL_LEFT;
	gButtonConfig[1].LongPress = 1;
	gButtonConfig[1].ShortPress = 1;
	gButtonConfig[1].ContinueLongPress = 1;

	gButtonConfig[2].ButtonAction = BUTTON_ACTION_FPGA;
	gButtonConfig[2].FPGAFunction.FPGAAction = FPGA_ACTION_SCROLL_UP;
	gButtonConfig[2].LongPress = 1;
	gButtonConfig[2].ShortPress = 1;
	gButtonConfig[2].ContinueLongPress = 1;

	gButtonConfig[3].ButtonAction = BUTTON_ACTION_FPGA;
	gButtonConfig[3].FPGAFunction.FPGAAction = FPGA_ACTION_SCROLL_DOWN;
	gButtonConfig[3].LongPress = 1;
	gButtonConfig[3].ShortPress = 1;
	gButtonConfig[3].ContinueLongPress = 1;

	gButtonConfig[4].ButtonAction = BUTTON_ACTION_REG;
	gButtonConfig[4].RegFunction.Chip = DEFAULT_CHIP;
	gButtonConfig[4].RegFunction.UsingMSBReg = 1;
	gButtonConfig[4].RegFunction.RegMSB = DEFAULT_REG_MSB;
	gButtonConfig[4].RegFunction.RegLSB = DEFAULT_REG_LSB;
	gButtonConfig[4].RegFunction.Step = DEFAULT_STEP;
	gButtonConfig[4].RegFunction.Sign = 1;
	gButtonConfig[4].LongPress = 1;
	gButtonConfig[4].ShortPress = 1;
	gButtonConfig[4].ContinueLongPress = 1;

	gButtonConfig[5].ButtonAction = BUTTON_ACTION_REG;
	gButtonConfig[5].RegFunction.Chip = DEFAULT_CHIP;
	gButtonConfig[5].RegFunction.UsingMSBReg = 1;
	gButtonConfig[5].RegFunction.RegMSB = DEFAULT_REG_MSB;
	gButtonConfig[5].RegFunction.RegLSB = DEFAULT_REG_LSB;
	gButtonConfig[5].RegFunction.Step = DEFAULT_STEP;
	gButtonConfig[5].RegFunction.Sign = 0;
	gButtonConfig[5].LongPress = 1;
	gButtonConfig[5].ShortPress = 1;
	gButtonConfig[5].ContinueLongPress = 1;

	gButtonConfig[6].ButtonAction = BUTTON_ACTION_SAVE;
	gButtonConfig[6].LongPress = 1;
	gButtonConfig[6].ShortPress = 0;
	gButtonConfig[6].ContinueLongPress = 0;
}

uint8_t AMIV_BUTTON_SaveGeneralConfig()
{
	return SaveGeneralConfig();
}

void AMIV_BUTTON_ConfigureButton(ButtonConfig_t *ButtonConfig_p, Button_t Button)
{
	if(Button > (NUMBER_OF_BUTTONS - 1))
	{
		return;
	}

	gButtonConfig[Button] = *ButtonConfig_p;
}

void AMIV_BUTTON_RemoveButtonConfiguration(Button_t Button)
{
	uint32_t buttons[0x100];
	uint32_t i;

	/* first save all the configuration */
	for(i = 0; i < 0x100; i++)
	{
		buttons[i] = AMIV_FLASH_ReadButtonConfig(i);
	}

	/* then erase the relevant page in flash */
	AMIV_FLASH_Erase(PAGE_BUTTION_CONFIG);

	/* then mark the config data for the button as 'removed' */
	for(i = 0; i < BUTTION_CONFIG_SIZE; i++)
	{
		buttons[AddrFlashButtons[Button]] = 0xFFFFFFFF;
	}

	/* then write back the temporarily stored data */
	for(i = 0; i < 0x100; i++)
	{
		if(buttons[i] != 0xFFFFFFFF)
		{
			AMIV_FLASH_WriteButtonConfig(i, buttons[i]);
		}
	}
}

uint8_t AMIV_BUTTON_CheckSpecialState()
{
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET)
	{
		return 0;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == Bit_SET)
	{
		return 0;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == Bit_SET)
	{
		return 0;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == Bit_SET)
	{
		return 0;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6) == Bit_SET)
	{
		return 0;
	}
	else if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == Bit_SET)
	{
		return 0;
	}
	else if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == Bit_SET)
	{
		return 0;
	}

	return 1;
}

void AMIV_BUTTON_FSM()
{
	uint32_t i = 0;

	/* GPIO pins are active low */
	switch(gButtonState)
	{
	case BUTTON_STATE_INIT:
		gButtonState = BUTTON_STATE_IDLE;
		break;
	case BUTTON_STATE_IDLE:
		gButton = Scan();
		if(gButton != BUTTON_NONE)
		{
			gButtonState = BUTTON_STATE_PRESS;
		}
		break;
	case BUTTON_STATE_PRESS:
		gButtonCntPress++;
		if(gButtonCntPress >= BUTTON_LONG_PRESS)
		{
			gButtonState = BUTTON_STATE_LONG_PRESS;
		}

		if(Scan() == BUTTON_NONE)
		{
			gButtonState = BUTTON_STATE_RELEASE;
		}
		break;
	case BUTTON_STATE_LONG_PRESS:
		ExecuteButtonFunctionLongPress();
		if(Scan() == BUTTON_NONE || !gButtonConfig[gButton].ContinueLongPress)
		{
			gButtonState = BUTTON_STATE_IDLE;
			gButtonCntPress = 0;
		}
		for(i = 0; i < (BUTTON_DEBOUNCE_LIMIT_LONG_PRESS - BUTTON_LED_DELAY); i++);
		break;
	case BUTTON_STATE_RELEASE:
		if(gButtonCntPress <= BUTTON_MIN_VALID_PRESS)
		{
			gButtonState = BUTTON_STATE_IDLE;
		}
		else
		{
			ExecuteButtonFunctionShortPress();
			gButtonState = BUTTON_STATE_IDLE;
		}
		gButtonCntPress = 0;
		break;
	}
}
