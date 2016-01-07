#ifndef __AMIV_FLASH_INC_GUARD
#define __AMIV_FLASH_INC_GUARD

#define PAGE_GENERAL_CONFIG	0
#define PAGE_BUTTION_CONFIG	3
#define BUTTION_CONFIG_SIZE	0x10

void AMIV_FLASH_Init();
uint8_t AMIV_FLASH_Erase(uint8_t Page);
uint32_t AMIV_FLASH_Read(uint32_t Address);
uint8_t AMIV_FLASH_Write(uint32_t Address, uint32_t Data);
uint8_t AMIV_FLASH_ButtonConfigPresent(uint32_t Address);
uint8_t AMIV_FLASH_GeneralConfigPresent();
void AMIV_FLASH_ReadGeneralConfig(uint8_t *Chip_p, uint8_t *Reg_p, uint8_t *Val_p, uint32_t Offset);
uint8_t AMIV_FLASH_WriteGeneralConfig(uint32_t *Array_p, uint32_t Count);
uint32_t AMIV_FLASH_ReadButtonConfig(uint32_t Offset);
uint8_t AMIV_FLASH_WriteButtonConfig(uint32_t Offset, uint32_t Data);

static const uint32_t AddrFlashButtons[] =
{
		0x00, /* Config for button 1 */
		0x10, /* Config for button 2 */
		0x20, /* Config for button 3 */
		0x30, /* Config for button 4 */
		0x40, /* Config for button 5 */
		0x50, /* Config for button 6 */
		0x60  /* Config for button 7 */
};

#endif
