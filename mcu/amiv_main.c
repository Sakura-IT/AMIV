#include "stm32f0xx_gpio.h"
#include "amiv_main.h"
#include "amiv_config.h"
#include "amiv_uart.h"
#include "amiv_i2c.h"
#include "amiv_irq.h"
#include "amiv_adv7511.h"
#include "amiv_ad9984a.h"
#include "amiv_flash.h"
#include "amiv_button.h"
#include "amiv_util.h"
#include "amiv_fpga.h"

int main(void)
{
	uint32_t i = 0;

	AMIV_CONFIG_Init();
	AMIV_UART_Init();
	AMIV_UART_SendString("Initiating MCU...\r\n");


	AMIV_CONFIG_GPIO();
	AMIV_CONFIG_LED();

	AMIV_I2C_Init();
	AMIV_IRQ_Init();
	AMIV_FLASH_Init();
	AMIV_BUTTON_Init();

	/* Deactivate led */
	GPIO_ResetBits(GPIOA, GPIO_Pin_8);

	for(i = 0; i < MAIN_STARTUP_DELAY; i++);

	AMIV_UART_SendString("Done!\r\n");

	AMIV_UART_SendString("Initiating devices...\r\n");
	AMIV_ADV7511_Init();
	AMIV_AD9984A_Init(ad9984a_mode_amiga);
	AMIV_UART_SendString("Done!\r\n");

	AMIV_UART_SendString("Powering up devices...\r\n");
	AMIV_ADV7511_PowerUp();
	AMIV_UART_SendString("Done!\r\n");

	if(AMIV_BUTTON_CheckSpecialState())
	{
		uint8_t Status = 0;
		AMIV_UART_SendString("Special state detected!\r\n");
		AMIV_UART_SendString("Resetting to vendor default...\r\n");
		Status |= AMIV_FLASH_Erase(0);
		Status |= AMIV_FLASH_Erase(1);
		Status |= AMIV_FLASH_Erase(2);
		Status |= AMIV_FLASH_Erase(3);
		if(Status == 0)
		{
			AMIV_UART_SendString("Done!\r\n");
		}
		else
		{
			AMIV_UART_SendString("Something went wrong!!\r\n");
		}
	}

	/* Load configuration from flash if present */
	if(AMIV_FLASH_GeneralConfigPresent())
	{
		AMIV_I2C_Slave Chip;
		uint8_t Reg;
		uint8_t Val;

		AMIV_UART_SendString("Found general configuration on flash!\r\n");
		AMIV_UART_SendString("Loading general configuration...\r\n");

		for(i = 0; i < (AD9984A_WR_REG_LIST_COUNT + ADV7511_WR_REG_LIST_COUNT); i++)
		{
			AMIV_FLASH_ReadGeneralConfig((uint8_t *)&Chip, &Reg, &Val, i);

			switch(Chip)
			{
			case AMIV_I2C_SLAVE_AD9984A:
				/* switch to AD9984A */
				AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_AD9984A);
				AMIV_I2C_WR_Reg(Reg, Val);
				break;
			case AMIV_I2C_SLAVE_ADV7511:
				/* switch to ADV7511 */
				AMIV_I2C_ChangeSlave(AMIV_I2C_SLAVE_ADV7511);
				AMIV_I2C_WR_Reg(Reg, Val);
				break;
			default:
				AMIV_UART_SendString("No such device. Please erase configuration in flash memory (cmd 'e')!");
			}
		}

		AMIV_UART_SendString("Done!\r\n");
	}

	for(i = 0; i < NUMBER_OF_BUTTONS; i++)
	{
		if(AMIV_FLASH_ButtonConfigPresent(AddrFlashButtons[i]))
		{
			ButtonConfig_t ButtonConfig;

			AMIV_UART_SendString("Found button configuration on flash for button ");
			AMIV_UART_SendString(AMIV_UTIL_itoa(i + 1));
			AMIV_UART_SendString("\r\n");

			ButtonConfig.ButtonAction = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x00);

			switch(ButtonConfig.ButtonAction)
			{
			case BUTTON_ACTION_FPGA:
				ButtonConfig.FPGAFunction.FPGAAction = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x01);
				break;
			case BUTTON_ACTION_REG:
				ButtonConfig.RegFunction.Chip = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x01);
				ButtonConfig.RegFunction.UsingMSBReg = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x02);
				ButtonConfig.RegFunction.RegMSB = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x03);
				ButtonConfig.RegFunction.RegLSB = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x04);
				ButtonConfig.RegFunction.Sign = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x05);
				ButtonConfig.RegFunction.Step =  AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x06);
				break;
			case BUTTON_ACTION_SAVE:
				/* Nothing more needed */
				break;
			case BUTTON_ACTION_RESET:
				/* Nothing more needed */
				break;
			}

			ButtonConfig.ShortPress = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x08);
			ButtonConfig.LongPress = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x09);
			ButtonConfig.ContinueLongPress = AMIV_FLASH_ReadButtonConfig(AddrFlashButtons[i] + 0x0A);

			AMIV_BUTTON_ConfigureButton(&ButtonConfig, i);
		}
	}

	/* Reset configuration in fpga */
	AMIV_FPGA_Reset();

	/* Wait for picture */
	while(AMIV_AD9984A_HSYNCActive() == 0)
	{
		for(i = 0; i < POLLING_INTERVAL; i++);
	}

	AMIV_UART_SendString("Starting output from FPGA\r\n");
	AMIV_FPGA_StartOutput();
	AMIV_UART_SendString("Done!\r\n");

    while(true)
    {
    	AMIV_BUTTON_FSM();
    }
}
