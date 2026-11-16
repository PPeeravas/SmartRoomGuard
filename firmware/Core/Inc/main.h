#ifndef SMART_ROOM_GUARD_MAIN_H
#define SMART_ROOM_GUARD_MAIN_H

#include "stm32f4xx_hal.h"

#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define ULTRASONIC_TRIG_Pin GPIO_PIN_8
#define ULTRASONIC_TRIG_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_4
#define BUZZER_GPIO_Port GPIOB
#define REED_Pin GPIO_PIN_5
#define REED_GPIO_Port GPIOB
#define ULTRASONIC_ECHO_Pin GPIO_PIN_10
#define ULTRASONIC_ECHO_GPIO_Port GPIOB
#define ULTRASONIC_ECHO_EXTI_IRQn EXTI15_10_IRQn

void Error_Handler(void);

#endif
