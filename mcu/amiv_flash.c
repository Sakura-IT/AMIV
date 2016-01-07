/* Format for button configuration
 *
 * Button 1:
 * 0x00 ButtonAction [BUTTON_ACTION_FPGA = 0, BUTTON_ACTION_REG = 1, BUTTON_ACTION_SAVE = 2, BUTTON_ACTION_RESET = 3]
 * 0x08 ShortPress [set to 1 if action should be performed upon short press]
 * 0x09 LongPress [set to 1 if action should be performed upon long press]
 * 0x0A ContinueLongPress [set to 1 if action should be repeated upon long press]
 *
 * BUTTON_ACTION_FPGA
 * 0x01 FPGAAction [FPGA_ACTION_SCROLL_RIGHT = 0, FPGA_ACTION_SCROLL_LEFT = 1, FPGA_ACTION_SCROLL_UP = 2, FPGA_ACTION_SCROLL_DOWN = 3]
 *
 * BUTTON_ACTION_REG
 * 0x01 Chip [AD9984A = 0, ADV7511 = 1]
 * 0x02 UsingMSBReg [if MSB register is used, then set to 1, otherwise set to 0]
 * 0x03 RegMSB [MSB register, e.g. 0x21]
 * 0x04 RegLSB [LSB register, e.g. 0x21]
 * 0x05 Sign [should the register value be increased with "Step" set to 1, otherwise set to 0]
 * 0x06 Step [the quantity that should be added or subtracted to the register value]
 *
 * No extra information is needed when action is set to BUTTON_ACTION_SAVE or BUTTON_ACTION_RESET
 *
 * Button 2:
 * Add 0x10 offset to flash address
 *
 */

#include "stm32f0xx_flash.h"
#include "amiv_main.h"
#include "amiv_uart.h"
#include "amiv_flash.h"

/* Base address of the Flash sectors */
#define ADDR_FLASH_START		((uint32_t)0x08007000) /* Base @ of Sector 7, 4 Kbytes */
#define ADDR_FLASH_END     		((uint32_t)0x08008000) /* End of flash */

static uint32_t PageToAddress(uint8_t Page)
{
	uint32_t Address = 0;

	switch(Page)
	{
	case 0:
		Address = ADDR_FLASH_START;
		break;
	case 1:
		Address = ADDR_FLASH_START + 0x400;
		break;
	case 2:
		Address = ADDR_FLASH_START + 0x800;
		break;
	case 3:
		Address = ADDR_FLASH_START + 0xC00;
		break;
	default:
		AMIV_UART_SendString("No such page!\r\n");
	}

	return Address;
}

void AMIV_FLASH_Init()
{

}

uint8_t AMIV_FLASH_Erase(uint8_t Page)
{
	uint8_t Status = 0;
	/* Unlock the Flash to enable the flash control register access */
	FLASH_Unlock();

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);


	if(PageToAddress(Page) != 0)
	{
		/* Must erase the page where marker is at to change its value */
		if(FLASH_ErasePage(PageToAddress(Page)) != FLASH_COMPLETE)
		{
			/*
			 * Error occurred while sector erase.
			 * User can add here some code to deal with this error
			 */
			AMIV_UART_SendString("Erase flash failed!\r\n");
			Status = 1;
		}
	}
	else
	{
		Status = 1;
	}

	/*
	 * Lock the Flash to disable the flash control register access (recommended
	 * to protect the FLASH memory against possible unwanted operation)
	 */
	FLASH_Lock();

	return Status;
}

uint32_t AMIV_FLASH_Read(uint32_t Address)
{
	/* MCU will hang if trying to read strange address */
	if(Address >= 0x400)
	{
		return 0xDEADBEEF;
	}

	return *(__IO uint32_t*)(Address*4 + ADDR_FLASH_START);
}

uint8_t AMIV_FLASH_Write(uint32_t Address, uint32_t Data)
{
	uint8_t Status = 0;

	/* Unlock the Flash to enable the flash control register access */
	FLASH_Unlock();

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

	if(FLASH_ProgramWord((Address*4 + ADDR_FLASH_START), Data) != FLASH_COMPLETE)
	{
		/*
		 * Error occurred while writing data in Flash memory.
		 * User can add here some code to deal with this error
		 */
		AMIV_UART_SendString("Flash failed!\r\n");
		Status = 1;
	}

	/*
	 * Lock the Flash to disable the flash control register access (recommended
	 * to protect the FLASH memory against possible unwanted operation)
	 */
	FLASH_Lock();

	return Status;
}

uint8_t AMIV_FLASH_ButtonConfigPresent(uint32_t Address)
{
	/* Configuration is stored at address 0x08007C00 - 0x08007DC0 in MCU */
	if(*(__IO uint32_t*)(Address*4 + PageToAddress(PAGE_BUTTION_CONFIG)) != 0xFFFFFFFF)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t AMIV_FLASH_GeneralConfigPresent()
{
	if(*(__IO uint32_t*)(PageToAddress(PAGE_GENERAL_CONFIG)) != 0xFFFFFFFF)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void AMIV_FLASH_ReadGeneralConfig(uint8_t *Chip_p, uint8_t *Reg_p, uint8_t *Val_p, uint32_t Offset)
{
	uint32_t V;

	/* MCU will hang if trying to read strange address */
	if(Offset >= 0x100) /* Only have 256 uint32, i.e one whole page 1024 bytes */
	{
		return;
	}

	V = *(__IO uint32_t*)(Offset*4 + PageToAddress(PAGE_GENERAL_CONFIG));

	*Chip_p = (V >> 24) & 0xFF;
	*Reg_p = (V >> 16) & 0xFF;
	*Val_p = V & 0xFF;
}

uint8_t AMIV_FLASH_WriteGeneralConfig(uint32_t *Array_p, uint32_t Count)
{
	uint32_t Address = 0;
	uint8_t Status = 0;

	/* Unlock the Flash to enable the flash control register access */
	FLASH_Unlock();

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

	Address = PageToAddress(PAGE_GENERAL_CONFIG);

	/* Must erase the page to change flash memory */
	if(FLASH_ErasePage(Address) != FLASH_COMPLETE)
	{
		/*
		 * Error occurred while sector erase.
		 * User can add here some code to deal with this error
		 */
		AMIV_UART_SendString("Erase flash failed!\r\n");
		Status = 1;
	}

	/* Program the user Flash area word by word */
	while(Address < ADDR_FLASH_END && Count > 0)
	{
		/*
		 * It is good to store the last array member first, assuming that this is where the
		 * last/highest register and value is stored.
		 * Remember that some registers (16bit wide) are only updated when LSB are written
		 * and normally the lower of two 8 bit register is LSB. So when we read from flash
		 * and then write to I2C registers we will write LSB last.
		 */
		if(FLASH_ProgramWord(Address, Array_p[Count - 1]) == FLASH_COMPLETE)
		{
			Address = Address + 4;
		}
		else
		{
			/*
			 * Error occurred while writing data in Flash memory.
			 * User can add here some code to deal with this error
			 */
			AMIV_UART_SendString("Flash failed!\r\n");
			Status = 1;
			break;
		}

		Count--;
	}

	if(Address == ADDR_FLASH_END)
	{
		AMIV_UART_SendString("Flash Incomplete!\r\n");
		Status = 1;
	}

	/*
	 * Lock the Flash to disable the flash control register access (recommended
	 * to protect the FLASH memory against possible unwanted operation)
	 */
	FLASH_Lock();

	return Status;
}

uint32_t AMIV_FLASH_ReadButtonConfig(uint32_t Offset)
{
	/* MCU will hang if trying to read strange address */
	if(Offset >= 0x100) /* Only have 256 uint32, i.e one whole page 1024 bytes */
	{
		return 0xDEADBEEF;
	}

	return *(__IO uint32_t*)(Offset*4 + PageToAddress(PAGE_BUTTION_CONFIG));
}

uint8_t AMIV_FLASH_WriteButtonConfig(uint32_t Offset, uint32_t Data)
{
	uint8_t Status = 0;

	/* Unlock the Flash to enable the flash control register access */
	FLASH_Unlock();

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

	if(FLASH_ProgramWord((Offset*4 + PageToAddress(PAGE_BUTTION_CONFIG)), Data) != FLASH_COMPLETE)
	{
		/*
		 * Error occurred while writing data in Flash memory.
		 * User can add here some code to deal with this error
		 */
		AMIV_UART_SendString("Flash failed!\r\n");
		Status = 1;
	}

	/*
	 * Lock the Flash to disable the flash control register access (recommended
	 * to protect the FLASH memory against possible unwanted operation)
	 */
	FLASH_Lock();

	return Status;
}
