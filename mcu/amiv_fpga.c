/* FPGA scroll truth table
 *
 * gpio_1	gpio_2	gpio_3	action
 *
 * 0		0		0		reset to default
 * 1		0		0		scroll right
 * 0		1		0		scroll up
 * 1		1		0		scroll left
 * 0		0		1		dont care
 * 1		0		1		dont care
 * 0		1		1		scroll down
 * 1		1		1		start output
 *
 * use gpio_0 to clock in to FPGA
 */

#include "stm32f0xx_gpio.h"
#include "amiv_main.h"

static void ClockOut()
{
	uint32_t i;

	/* Toggle gpio 0 to clock the setting into FPGA */
	GPIO_SetBits(GPIOB, GPIO_Pin_5); /* FPGA gpio_0 */
	for(i = 0; i < MAIN_DELAY; i++);
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);
	for(i = 0; i < MAIN_DELAY; i++);
	GPIO_SetBits(GPIOB, GPIO_Pin_5);
	for(i = 0; i < MAIN_DELAY; i++);
}

void AMIV_FPGA_StartOutput()
{
	GPIO_SetBits(GPIOA, GPIO_Pin_15); /* FPGA gpio_3 */
	GPIO_SetBits(GPIOB, GPIO_Pin_3); /* FPGA gpio_2 */
	GPIO_SetBits(GPIOB, GPIO_Pin_4); /* FPGA gpio_1 */

	ClockOut();

	/* Activate MCU led */
	GPIO_SetBits(GPIOA, GPIO_Pin_8);
}

void AMIV_FPGA_Reset()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_15); /* FPGA gpio_3 */
	GPIO_ResetBits(GPIOB, GPIO_Pin_3); /* FPGA gpio_2 */
	GPIO_ResetBits(GPIOB, GPIO_Pin_4); /* FPGA gpio_1 */

	ClockOut();
}

void AMIV_FPGA_ScrollRight()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_15); /* FPGA gpio_3 */
	GPIO_ResetBits(GPIOB, GPIO_Pin_3); /* FPGA gpio_2 */
	GPIO_SetBits(GPIOB, GPIO_Pin_4); /* FPGA gpio_1 */

	ClockOut();
}

void AMIV_FPGA_ScrollLeft()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_15); /* FPGA gpio_3 */
	GPIO_SetBits(GPIOB, GPIO_Pin_3); /* FPGA gpio_2 */
	GPIO_SetBits(GPIOB, GPIO_Pin_4); /* FPGA gpio_1 */

	ClockOut();
}

void AMIV_FPGA_ScrollUp()
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_15); /* FPGA gpio_3 */
	GPIO_SetBits(GPIOB, GPIO_Pin_3); /* FPGA gpio_2 */
	GPIO_ResetBits(GPIOB, GPIO_Pin_4); /* FPGA gpio_1 */

	ClockOut();
}

void AMIV_FPGA_ScrollDown()
{
	GPIO_SetBits(GPIOA, GPIO_Pin_15); /* FPGA gpio_3 */
	GPIO_SetBits(GPIOB, GPIO_Pin_3); /* FPGA gpio_2 */
	GPIO_ResetBits(GPIOB, GPIO_Pin_4); /* FPGA gpio_1 */

	ClockOut();
}

