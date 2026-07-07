#include "main.h"
#include "stm32_timer.h"
#include "stm32l0xx_hal_conf.h"

extern TIM_HandleTypeDef htim6;
void MX_TIM6_Init(void);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base);
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base);
