#include "main.h"

extern UART_HandleTypeDef huart1;

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(ULTRASONIC_ECHO_Pin);
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}
