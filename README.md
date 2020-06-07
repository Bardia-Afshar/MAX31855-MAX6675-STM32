# MAX6675-STM32
Library for MAX6675 from Maxim Integrated for STM32 

## Introduction

For sensing a temperature in my hot air gun nozzle, I had to use an amplifier circuit to amplify output signal of a thermocouple. As you probably know, MAX6675 is an all-in-one Integrated circuit that provides a digital output from thermocouple input. As a result, you can save some components and space on your PCB. According to ‘Datasheet’, The MAX6675 performs cold-junction compensation and digitizes the signal from a type-K thermocouple. The data is output in a 12-bit resolution, SPI compatible, read-only format. I wrote a simple library that you can use to get the temperature from MAX6675. I used STM32F030F4P6. However, you can port it to other microcontrollers. I also provided a complete project in STM32CubeIDE that you can use as a reference. You can run code and see data in STMStudio.

[Download Datasheet Here](https://datasheets.maximintegrated.com/en/ds/MAX6675.pdf)

![IC](https://user-images.githubusercontent.com/54714609/83968251-18785700-a8dd-11ea-89c8-dec998a6625f.jpg)


## How to use

1- Activate SPI from STM32CubeMX. As I mentioned before, this chip is read-only, so that you can configure your microcontroller as a receive-only master. Also, the length of data is 16bit and MSB First. Use one of the GPIO’s as a Slave Select(SS). I used first pin from GPIOB (GPIOB_PIN_1).

![uC](https://user-images.githubusercontent.com/54714609/83968282-46f63200-a8dd-11ea-911a-a031615f12c3.jpg)
![SPI-Config](https://user-images.githubusercontent.com/54714609/83968280-452c6e80-a8dd-11ea-8e9e-3bd5b2b64908.jpg)

2-Add my library to your compiler and in MAX6675.h set the SS port and pin that you activated in STM32cubeMX. The output of Max6675_Read_Temp is the temperature in centigrade

```c
#define SSPORT GPIOB       // GPIO Port of Chip Select(Slave Select)
#define SSPIN  GPIO_PIN_1  // GPIO PIN of Chip Select(Slave Select)
```

## Results

You can see the test results of this chip. I put thermocouple in the ice to plot the temperature change. TS is connection flag. If you disconnect thermocouple from the chip, it goes high.

![MAX6675](https://user-images.githubusercontent.com/54714609/83968334-b3713100-a8dd-11ea-9030-8786ef1221f8.gif)
