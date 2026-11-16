#include "main.h"

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *timer)
{
    if (timer->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}
