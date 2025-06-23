# vu-meter

Physical VU meter for use with rack audio equipment

Consists of:
- 1 processor board
- 4 indicator boards (2 boards per channel)

## Processor board

The processor board is a simple board with converter from balanced input to unbalanced with 
2 x CH32X035F7P6 MCU to convert from analog domain to digital for output to SPI for shift registers.

## Indicator board

This board consists of 5 74HC595D shift registers for 35 individual LEDs.