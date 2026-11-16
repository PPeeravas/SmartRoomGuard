# Hardware Notes

Draft pin map. Confirm in CubeMX before wiring.

| Function | Pin |
|---|---|
| UART TX → NodeMCU RX | PA9 (USART1) |
| UART RX ← NodeMCU TX | PA10 (USART1) |
| Reed switch | PB5 |
| Ultrasonic TRIG | PA8 |
| Ultrasonic ECHO | PB10 (5 V-tolerant) |
| Buzzer | PB4 |

- Avoid PA2/PA3 (USART2, used by the ST-Link USB).
- Phone hotspot must be 2.4 GHz for the ESP32.
- All grounds connected together.

# Specification
STM32 board = NUCLEO-F411RE \
NodeMCU = ESP32-S3-N16R8