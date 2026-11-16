# Hardware Notes

STM32 firmware pin map. USART1 is the NodeMCU link and USART2 is the debug port.

| Function                    | Pin                                                  |
| --------------------------- | ---------------------------------------------------- |
| UART TX → NodeMCU RX        | PA9 (USART1)                                         |
| UART RX ← NodeMCU TX        | PA10 (USART1)                                        |
| Reed switch (KY-021) signal | PB5, input pull-up                                   |
| Ultrasonic TRIG             | PA8                                                  |
| Ultrasonic ECHO             | PB10, EXTI on both edges, no internal pull           |
| Buzzer (KY-012) control     | PB4, active high, low at startup                     |
| Onboard LD2                 | PA5, shows the alarm state                           |
| Onboard B1                  | PC13, active-low arm/disarm button                   |
| Debug serial TX             | PA2 (USART2), routed to the ST-LINK Virtual COM Port |

- Connect the [HC-SR04](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf)
  to 5 V and common ground. ST specifies PB10 as a
  [5 V-tolerant input](https://www.st.com/resource/en/datasheet/stm32f411re.pdf).
- Wire the KY-021 so the magnet closes the signal contact to ground. If the
  module uses a supply pin for its pull-up, connect that pin to 3.3 V.
- TIM2 runs at 1 MHz from the 16 MHz internal clock with prescaler 15. The
  firmware sends a 12 µs trigger every 70 ms and uses the ECHO edges to time
  the pulse.
- Use the ST-LINK USB cable for debug serial at 115200 baud, 8N1. The NodeMCU
  link on PA9/PA10 also runs at 115200 baud, 8N1.
- Phone hotspot must be 2.4 GHz for the ESP8266.
- All grounds connected together.

# Specification

STM32 board = NUCLEO-F411RE \
NodeMCU = ESP32-S3-N16R8
