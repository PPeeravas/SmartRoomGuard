#include "main.h"

/* USER CODE BEGIN Includes */
#include "sensors.h"
/* USER CODE END Includes */

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
/* Inspect this snapshot in CubeIDE Live Expressions during sensor bring-up. */
static volatile SensorReadings sensorReadings = { .distanceCm = -1 };
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

int main(void)
{
	HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();

    /* USER CODE BEGIN 2 */
    if (Sensors_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE END 2 */

    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        SensorReadings current = Sensors_Update();
        sensorReadings = current;
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin,
                          current.doorOpen ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
    /* USER CODE END 3 */
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef oscillator = {0};
    oscillator.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    oscillator.HSIState = RCC_HSI_ON;
    oscillator.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    oscillator.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&oscillator) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitTypeDef clock = {0};
    clock.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                      RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clock.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clock.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clock.APB1CLKDivider = RCC_HCLK_DIV1;
    clock.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clock, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_TIM2_Init(void)
{
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 15; /* 16 MHz / 16 = 1 MHz */
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = UINT32_MAX;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOA, LD2_Pin | ULTRASONIC_TRIG_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

    gpio.Pin = LD2_Pin | ULTRASONIC_TRIG_Pin;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = BUZZER_Pin;
    HAL_GPIO_Init(BUZZER_GPIO_Port, &gpio);

    gpio.Pin = REED_Pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(REED_GPIO_Port, &gpio);

    gpio.Pin = ULTRASONIC_ECHO_Pin;
    gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ULTRASONIC_ECHO_GPIO_Port, &gpio);

    HAL_NVIC_SetPriority(ULTRASONIC_ECHO_EXTI_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(ULTRASONIC_ECHO_EXTI_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    if (pin == ULTRASONIC_ECHO_Pin)
    {
        Sensors_OnEchoEdge();
    }
}
/* USER CODE END 4 */

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}
