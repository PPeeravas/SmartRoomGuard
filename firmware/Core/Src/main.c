#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "sensors.h"
/* USER CODE END Includes */

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
enum
{
    ARM_DELAY_MS = 10000, //10 seconds to leave
    BUTTON_DEBOUNCE_MS = 25, // Debounce for B1 button
    BUZZER_BEEP_MS = 40,
    BUZZER_PERIOD_MS = 1000,
    STATUS_INTERVAL_MS = 2000,
    SERIAL_TX_TIMEOUT_MS = 10,
    COMMAND_RX_CAPACITY = 64
};

typedef enum
{
    DISARMED,
    ARMING,
    ARMED,
    ALARM,
    SILENCED
} AlarmState;

typedef enum
{
    NO_COMMAND,
    ARM_COMMAND,
    DISARM_COMMAND,
    SILENCE_COMMAND
} AlarmCommand;

static const char *const stateNames[] = {
    [DISARMED] = "DISARMED", [ARMING] = "ARMING", [ARMED] = "ARMED",
    [ALARM] = "ALARM", [SILENCED] = "SILENCED"
};

/* Inspect this snapshot in CubeIDE Live Expressions during sensor bring-up. */
static volatile SensorReadings sensorReadings = { .distanceCm = -1 };
static uint8_t receivedByte;
static volatile uint8_t commandRx[COMMAND_RX_CAPACITY];
static volatile uint8_t commandWrite;
static volatile uint8_t commandRead;
static volatile bool commandRxLost;
/* USER CODE END PV */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);

/* USER CODE BEGIN PFP */
static bool ButtonPressed(uint32_t nowMs);
static AlarmCommand ReadCommand(void);
static AlarmState ApplyCommand(AlarmState state, AlarmCommand command);
static void UpdateIndicators(AlarmState state, uint32_t elapsedMs);
static void ReportStatus(AlarmState state, SensorReadings readings);
/* USER CODE END PFP */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM2_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    /* USER CODE BEGIN 2 */
    if (Sensors_Init(&htim2) != HAL_OK ||
        HAL_UART_Receive_IT(&huart1, &receivedByte, 1) != HAL_OK)
    {
        Error_Handler();
    }

    SensorReadings previous = Sensors_Update();
    AlarmState state = DISARMED;
    uint32_t stateStartedMs = HAL_GetTick();
    uint32_t lastReportMs = stateStartedMs - STATUS_INTERVAL_MS;
    /* USER CODE END 2 */

    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        SensorReadings current = Sensors_Update();
        sensorReadings = current;
        uint32_t nowMs = HAL_GetTick();
        AlarmState next = state;

        if (state == ARMED && current.doorOpen && !previous.doorOpen)
        {
            next = ALARM;
        }
        else if (state == ARMING && nowMs - stateStartedMs >= ARM_DELAY_MS)
        {
            next = current.doorOpen ? ALARM : ARMED;
        }

        if (ButtonPressed(nowMs))
        {
            next = ApplyCommand(next, next == DISARMED ? ARM_COMMAND : DISARM_COMMAND);
        }
        next = ApplyCommand(next, ReadCommand());

        bool stateChanged = next != state;
        if (stateChanged)
        {
            state = next;
            stateStartedMs = nowMs;
        }
        UpdateIndicators(state, nowMs - stateStartedMs);

        bool statusDue = nowMs - lastReportMs >= STATUS_INTERVAL_MS;
        if (statusDue || stateChanged || current.doorOpen != previous.doorOpen ||
            current.distanceCm != previous.distanceCm)
        {
            ReportStatus(state, current);
        }
        if (statusDue)
        {
            lastReportMs = nowMs;
        }
        previous = current;
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

static void InitUart(UART_HandleTypeDef *uart, USART_TypeDef *instance, uint32_t mode)
{
    uart->Instance = instance;
    uart->Init.BaudRate = 115200;
    uart->Init.WordLength = UART_WORDLENGTH_8B;
    uart->Init.StopBits = UART_STOPBITS_1;
    uart->Init.Parity = UART_PARITY_NONE;
    uart->Init.Mode = mode;
    uart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart->Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(uart) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_USART1_UART_Init(void)
{
    InitUart(&huart1, USART1, UART_MODE_TX_RX);
}

static void MX_USART2_UART_Init(void)
{
    InitUart(&huart2, USART2, UART_MODE_TX);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

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

    gpio.Pin = USER_BUTTON_Pin;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &gpio);

    gpio.Pin = ULTRASONIC_ECHO_Pin;
    gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ULTRASONIC_ECHO_GPIO_Port, &gpio);

    HAL_NVIC_SetPriority(ULTRASONIC_ECHO_EXTI_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(ULTRASONIC_ECHO_EXTI_IRQn);
}

/* USER CODE BEGIN 4 */
static bool ButtonPressed(uint32_t nowMs)
{
    static bool lastRawPressed;
    static bool stablePressed;
    static uint32_t changedMs;
    bool rawPressed = HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_RESET;

    if (rawPressed != lastRawPressed)
    {
        lastRawPressed = rawPressed;
        changedMs = nowMs;
    }
    if (rawPressed != stablePressed && nowMs - changedMs >= BUTTON_DEBOUNCE_MS)
    {
        stablePressed = rawPressed;
        return stablePressed;
    }
    return false;
}

static AlarmCommand ReadCommand(void)
{
    static char line[sizeof "C,SILENCE\r"];
    static size_t length;
    static bool discardLine;

    while (commandRxLost || commandRead != commandWrite)
    {
        uint32_t primask = __get_PRIMASK();
        __disable_irq();
        if (commandRxLost)
        {
            commandRead = commandWrite;
            commandRxLost = false;
            discardLine = true;
            __set_PRIMASK(primask);
            continue;
        }
        uint8_t byte = commandRx[commandRead];
        commandRead = (commandRead + 1) % COMMAND_RX_CAPACITY;
        __set_PRIMASK(primask);

        if (byte == '\n')
        {
            AlarmCommand command = NO_COMMAND;
            if (!discardLine)
            {
                if (length > 0 && line[length - 1] == '\r')
                {
                    --length;
                }
                line[length] = '\0';
                if (strcmp(line, "C,ARM") == 0)
                {
                    command = ARM_COMMAND;
                }
                else if (strcmp(line, "C,DISARM") == 0)
                {
                    command = DISARM_COMMAND;
                }
                else if (strcmp(line, "C,SILENCE") == 0)
                {
                    command = SILENCE_COMMAND;
                }
            }
            length = 0;
            discardLine = false;
            if (command != NO_COMMAND)
            {
                return command;
            }
        }
        else if (length == sizeof line - 1 || byte == '\0')
        {
            discardLine = true;
        }
        else if (!discardLine)
        {
            line[length++] = (char)byte;
        }
    }
    return NO_COMMAND;
}

static AlarmState ApplyCommand(AlarmState state, AlarmCommand command)
{
    if (command == ARM_COMMAND && state == DISARMED)
    {
        return ARMING;
    }
    if (command == DISARM_COMMAND && (state == ARMED || state == ALARM || state == SILENCED))
    {
        return DISARMED;
    }
    if (command == SILENCE_COMMAND && state == ALARM)
    {
        return SILENCED;
    }
    return state;
}

static void UpdateIndicators(AlarmState state, uint32_t elapsedMs)
{
    bool ledOn = false;
    switch (state)
    {
        case DISARMED: break;
        case ARMING:   ledOn = elapsedMs % 500 < 250; break;
        case ARMED:    ledOn = true; break;
        case ALARM:    ledOn = elapsedMs % 200 < 100; break;
        case SILENCED: ledOn = elapsedMs % 2000 < 1000; break;
    }
    bool buzzerOn = state == ALARM && elapsedMs % BUZZER_PERIOD_MS < BUZZER_BEEP_MS;
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, ledOn ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, buzzerOn ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void ReportStatus(AlarmState state, SensorReadings readings)
{
    char message[64];
    int length = snprintf(message, sizeof message, "S,%s,%u,%ld\n",
                          stateNames[state], (unsigned)readings.doorOpen, (long)readings.distanceCm);
    if (length > 0 && (size_t)length < sizeof message)
    {
        HAL_UART_Transmit(&huart1, (const uint8_t *)message, (uint16_t)length, SERIAL_TX_TIMEOUT_MS);
    }

    length = snprintf(message, sizeof message, "state=%s door=%s distance_cm=%ld\r\n",
                      stateNames[state], readings.doorOpen ? "OPEN" : "CLOSED", (long)readings.distanceCm);
    if (length > 0 && (size_t)length < sizeof message)
    {
        HAL_UART_Transmit(&huart2, (const uint8_t *)message, (uint16_t)length, SERIAL_TX_TIMEOUT_MS);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart)
{
    if (uart->Instance == USART1)
    {
        uint8_t nextWrite = (commandWrite + 1) % COMMAND_RX_CAPACITY;
        if (nextWrite == commandRead)
        {
            commandRxLost = true;
        }
        else if (!commandRxLost)
        {
            commandRx[commandWrite] = receivedByte;
            commandWrite = nextWrite;
        }
        HAL_UART_Receive_IT(uart, &receivedByte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart)
{
    if (uart->Instance == USART1)
    {
        commandRxLost = true;
        if (uart->RxState == HAL_UART_STATE_READY)
        {
            HAL_UART_Receive_IT(uart, &receivedByte, 1);
        }
    }
}

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
