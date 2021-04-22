/*
 * my_function.c
 *
 *  Created on: Apr 22, 2021
 *      Author: bill.wang
 *
 *
 *  -- PA8(TIM1_CH1) 	: hall period counter time base 10us
 *  -- PA15(TIM2_CH1) 	: PWM output
 *  -- PB12/13/14		: Hall output HA/HB/HC
 *  -- PA0				: Button
 */
#include "main.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
// Define states
enum FSMSTATE
{
	S0, S1, S2, S3, S4, S5, Sx
} hall_state;

/* Variables */
volatile uint32_t cnt_hall_period = 0;
volatile uint32_t cnt_10us = 0;
volatile uint32_t hall_period = 0, hall_60d = 0, hall_120d = 0, hall_180d = 0,
		hall_240d = 0, hall_300d = 0;
volatile uint32_t cnt_pwm_period = 0, pwm_period = 0, pwm_duty = 0;

/* define GPIO mapping */
typedef struct
{
	GPIO_TypeDef *port;
	uint32_t pin;
} GPIO_MAP;

GPIO_MAP gpio_map[] =
{
{ HA_GPIO_Port, HA_Pin },
{ HB_GPIO_Port, HB_Pin },
{ HC_GPIO_Port, HC_Pin },
{ B1_GPIO_Port, B1_Pin }, };

void SetOutput(uint8_t ch, uint8_t hl)
{

	if (hl == 0)
	{
		gpio_map[ch].port->BRR = (uint32_t) gpio_map[ch].pin;
	}
	else
	{
		gpio_map[ch].port->BSRR = (uint32_t) gpio_map[ch].pin;
	}
}

uint8_t GetInput(uint8_t ch)
{
	return ((gpio_map[ch].port)->IDR & (gpio_map[ch].pin)) == 0 ? 0 : 1;
}
/*
 set hall_period variable
 period取6的倍數, 週期= period x 10us, 498=>200Hz, 996 => 100Hz
 */
void Hall_Period_Init(uint32_t period)
{
	hall_period = period;
	hall_60d = hall_period / 6;
	hall_120d = hall_period / 3;
	hall_180d = hall_period / 2;
	hall_240d = hall_120d << 1;
	hall_300d = hall_180d + hall_120d;
}

void main_init()
{
	hall_state = S5;  // initial state = S5
	cnt_10us = 100000; // 1sec for low frequency
	Hall_Period_Init(9960); // set hall at low frequency 10Hz

	////// wait for user button pushed
	while (1)
	{
		if (GetInput(3) == 0)
		{
			HAL_Delay(100); // debounce
			if (GetInput(3) == 0)
			{
				break;
			}
		}
	}

	////// enable timer
	__HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

// update event
//put this code into HAL_TIM_PeriodElapsedCallback
void timer_period_elasped_callback(TIM_HandleTypeDef *htim)
{
	if (cnt_10us > 0)
		cnt_10us--;

	if (++cnt_hall_period >= hall_period)
		cnt_hall_period = 0;

	switch (hall_state)
	{
	case S0:
		if (cnt_hall_period == (hall_60d - 1))
		{
			SetOutput(0, 1);
			SetOutput(1, 0);
			SetOutput(2, 0);
			hall_state = S1;
		}
		break;
	case S1:
		if (cnt_hall_period == (hall_120d - 1))
		{
			SetOutput(0, 1);
			SetOutput(1, 1);
			SetOutput(2, 0);
			hall_state = S2;
		}
		break;
	case S2:
		if (cnt_hall_period == (hall_180d - 1))
		{
			SetOutput(0, 0);
			SetOutput(1, 1);
			SetOutput(2, 0);
			hall_state = S3;
		}
		break;
	case S3:
		if (cnt_hall_period == (hall_240d - 1))
		{
			SetOutput(0, 0);
			SetOutput(1, 1);
			SetOutput(2, 1);
			hall_state = S4;
		}
		break;
	case S4:
		if (cnt_hall_period == (hall_300d - 1))
		{
			SetOutput(0, 0);
			SetOutput(1, 0);
			SetOutput(2, 1);
			hall_state = S5;
		}
		break;
	case S5:
		if (cnt_hall_period == (hall_period - 1))
		{
			SetOutput(0, 1);
			SetOutput(1, 0);
			SetOutput(2, 1);
			hall_state = S0;
		}
		break;
	default:
		SetOutput(0, 0);
		SetOutput(1, 0);
		SetOutput(2, 0);
		break;
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	timer_period_elasped_callback(htim);
}
