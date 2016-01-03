#include "stm32f0xx_flash.h"
#include "amiv_uart.h"

/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 4 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08001000) /* Base @ of Sector 1, 4 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08002000) /* Base @ of Sector 2, 4 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x08003000) /* Base @ of Sector 3, 4 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08004000) /* Base @ of Sector 4, 4 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08005000) /* Base @ of Sector 5, 4 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08006000) /* Base @ of Sector 6, 4 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08007000) /* Base @ of Sector 7, 4 Kbytes */
#define ADDR_FLASH_END     		((uint32_t)0x08008000) /* End of flash */

#define ADDR_FLASH_MARKER		((uint32_t)0x08007C00) /* Flash marker */

void AMIV_FLASH_Init()
{
}

void AMIV_FLASH_program()
{
	uint32_t Address = 0;

	/* Unlock the Flash to enable the flash control register access */
	FLASH_Unlock();

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

	/* Program the user Flash area word by word */

	Address = ADDR_FLASH_MARKER;

	/* First write marker, to know if the data in flash should be used or not */
	if(FLASH_ProgramWord(Address, 0xDEADBEEF) != FLASH_COMPLETE)
	{
		/*
		 * Error occurred while writing data in Flash memory.
		 * User can add here some code to deal with this error
		 */
		AMIV_UART_SendString("Flash failed!\r\n");
	}

	while(Address < ADDR_FLASH_END)
	{
		if(FLASH_ProgramWord(Address, 0x1) == FLASH_COMPLETE)
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
			break;
		}
	}

	/*
	 * Lock the Flash to disable the flash control register access (recommended
	 * to protect the FLASH memory against possible unwanted operation)
	 */
	FLASH_Lock();
}

uint32_t AMIV_FLASH_read(uint32_t Address)
{
	return *(__IO uint32_t*)(Address + ADDR_FLASH_SECTOR_7);
}

uint8_t AMIV_FLASH_write(uint32_t Address, uint32_t Data)
{
	uint8_t res = 0;
	/* Unlock the Flash to enable the flash control register access */
	FLASH_Unlock();

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

	if(FLASH_ProgramWord((Address + ADDR_FLASH_SECTOR_7), Data) != FLASH_COMPLETE)
	{
		/*
		 * Error occurred while writing data in Flash memory.
		 * User can add here some code to deal with this error
		 */
		AMIV_UART_SendString("Flash failed!\r\n");
		res = 1;
	}

	/*
	 * Lock the Flash to disable the flash control register access (recommended
	 * to protect the FLASH memory against possible unwanted operation)
	 */
	FLASH_Lock();

	return res;
}

uint8_t AMIV_FLASH_marker(uint8_t on_off) /* on_off as 0 equals OFF */
{
	uint8_t res = 0;

	/* Unlock the Flash to enable the flash control register access */
	FLASH_Unlock();

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP);

	/* must erase the page where marker is at to change its value */
	if(FLASH_ErasePage(ADDR_FLASH_MARKER) != FLASH_COMPLETE)
	{
		/*
		 * Error occurred while sector erase.
		 * User can add here some code to deal with this error
		 */
		AMIV_UART_SendString("Erase flash failed!\r\n");
	}

	if(on_off == 1) /* e.i. ON */
	{
		if(FLASH_ProgramWord((ADDR_FLASH_MARKER), 0xDEADBEEF) != FLASH_COMPLETE)
		{
			/*
			 * Error occurred while writing data in Flash memory.
			 * User can add here some code to deal with this error
			 */
			AMIV_UART_SendString("Flash failed!\r\n");
			res = 1;
		}
	}
	else
	{
		if(FLASH_ProgramWord((ADDR_FLASH_MARKER), 0x00000000) != FLASH_COMPLETE)
		{
			/*
			 * Error occurred while writing data in Flash memory.
			 * User can add here some code to deal with this error
			 */
			AMIV_UART_SendString("Flash failed!\r\n");
			res = 1;
		}
	}

	/*
	 * Lock the Flash to disable the flash control register access (recommended
	 * to protect the FLASH memory against possible unwanted operation)
	 */
	FLASH_Lock();

	return res;
}
