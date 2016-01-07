#ifndef __AMIV_BUTTON_INC_GUARD
#define __AMIV_BUTTON_INC_GUARD

typedef enum
{
	BUTTON_1,
	BUTTON_2,
	BUTTON_3,
	BUTTON_4,
	BUTTON_5,
	BUTTON_6,
	BUTTON_7,
	BUTTON_NONE
}Button_t;

typedef enum
{
	BUTTON_ACTION_FPGA,
	BUTTON_ACTION_REG,
	BUTTON_ACTION_SAVE,
	BUTTON_ACTION_RESET,
}ButtonAction_t;

typedef enum
{
	FPGA_ACTION_SCROLL_RIGHT,
	FPGA_ACTION_SCROLL_LEFT,
	FPGA_ACTION_SCROLL_UP,
	FPGA_ACTION_SCROLL_DOWN,
}FPGAAction_t;

typedef struct
{
	FPGAAction_t FPGAAction;
} FPGAFunction_t;

typedef struct
{
	uint8_t Chip;
	uint8_t UsingMSBReg; /* Set to 1 will  mean that 2 registers are used */
	uint8_t RegMSB;
	uint8_t RegLSB;
	uint8_t Step;
	uint8_t Sign; /* Set to 1 will mean an addition and 0 mean substraction */
} RegFunction_t;

typedef struct
{
	ButtonAction_t ButtonAction;
	FPGAFunction_t FPGAFunction;
	RegFunction_t RegFunction;
	uint8_t LongPress;
	uint8_t ShortPress;
	uint8_t ContinueLongPress;
} ButtonConfig_t;

#define NUMBER_OF_BUTTONS				7

void AMIV_BUTTON_Init();
void AMIV_BUTTON_FSM();
uint8_t AMIV_BUTTON_SaveGeneralConfig();
void AMIV_BUTTON_ConfigureButton(ButtonConfig_t *ButtonConfig_p, Button_t Button);
void AMIV_BUTTON_RemoveButtonConfiguration(Button_t Button);
uint8_t AMIV_BUTTON_CheckSpecialState();

#endif
