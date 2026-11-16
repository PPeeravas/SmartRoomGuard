#ifndef SMART_ROOM_GUARD_SENSORS_H
#define SMART_ROOM_GUARD_SENSORS_H

#include <stdbool.h>
#include "stm32f4xx_hal.h"

typedef struct
{
    bool doorOpen;
    int32_t distanceCm; /* -1 until a valid echo is measured, or after a timeout. */
} SensorReadings;

/* Call after GPIO and the 1 MHz, 32-bit TIM2 counter have been configured. */
HAL_StatusTypeDef Sensors_Init(TIM_HandleTypeDef *timer);

/* Call from the main loop. Only the short trigger pulse waits on the timer. */
SensorReadings Sensors_Update(void);

/* Forward both edges of the ECHO pin here from the EXTI callback. */
void Sensors_OnEchoEdge(void);

#endif
