# MAX31855-MAX6675-STM32
Library for MAX31855/MAX6675 from Maxim Integrated for STM32 

STM32 (HAL) drivers and example projects for the MAX31855 and MAX6675 K-type thermocouple converters, with polling and DMA examples. 
Includes a MicroPython port.

Driver now supports polling and DMA modes, plus fault decoding. There are now 4 buildable STM32CubeIDE example projects.

## Why these chips

Both chips take a raw thermocouple signal and hand you a ready-to-use
temperature over SPI — no op-amp, no ADC, no cold-junction math of your own.

| | MAX31855 | MAX6675 |
|:---:|:---:|:---:|
| Resolution | 0.25 °C (14-bit signed) | 0.25 °C (12-bit unsigned) |
| Range | −270 °C to +1800 °C | 0 °C to +1024 °C |
| Cold-junction readout | Yes | No |
| Fault detection | Open, short-to-GND, short-to-VCC | Open circuit only |
| SPI frame | 32-bit | 16-bit |


## Repository layout

```
library/
  max31855/            Standalone MAX31855 HAL driver (max31855.c / .h)
  max6675/             Standalone MAX6675 HAL driver (max6675.c / .h)
  micropython/         MicroPython ports of both drivers (single-file classes)

examples/
  max31855_polling/    STM32CubeIDE project — MAX31855, blocking SPI
  max31855_dma/        STM32CubeIDE project — MAX31855, DMA reads + design notes
  max6675_polling/     STM32CubeIDE project — MAX6675, blocking SPI
  max6675_dma/         STM32CubeIDE project — MAX6675, DMA reads + design notes
```

Each example is a complete, importable STM32CubeIDE project (`.project`,
`.cproject`, `.ioc`, linker script) targeting an **STM32G431K**.

## Wiring

Both chips are read-only SPI slaves (mode 0, MSB-first, no MOSI):

| Signal | MCU pin | Notes |
|:---:|:---:|:---:|
| `SCK` | PA5 | SPI clock |
| `SO`/`MISO` | PA6 | Data out from the chip |
| `CS`/`SS` | PA7 | Software GPIO, driven by the driver ||


## Building & flashing

1. Install [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html).
2. `File → Open Projects from File System…` and point it at one of the
   `examples/*` folders.
3. Build (`Ctrl+B`) and flash onto an STM32G431-based board (or regenerate
   the `.ioc` for a different target).
4. Wire the sensor per [Wiring](#wiring), then check the readings — you can
   watch temperatures in a debugger, or use a tool like
   [MCUViewer](https://github.com/klonyyy/MCUViewer) to read the variable
   live, like in the screenshot below.
<img width="1920" height="1032" alt="Example" src="https://github.com/user-attachments/assets/c21ab673-2db7-44f1-9eb6-910f8d098832" />

