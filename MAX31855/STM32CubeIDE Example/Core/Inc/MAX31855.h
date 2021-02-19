/*************************************************************************************
 Title	 :  MAXIM Integrated MAX31855 Library for STM32 Using HAL Libraries
 Author  :  Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Software:  STM32CubeIDE
 Hardware:  Any STM32 device
*************************************************************************************/
#ifndef MAX31855_H_
#define MAX31855_H_
#include "main.h"
// ------------------------- Defines -------------------------
extern uint8_t Error;	   // Error Detection - 1-> No Connection / 2-> Short to GND / 4-> Short to VCC
#define SSPORT GPIOA       // GPIO Port of Chip Select(Slave Select)
#define SSPIN  GPIO_PIN_4  // GPIO PIN of Chip Select(Slave Select)
// ------------------------- Functions  ----------------------
float Max31855_Read_Temp(void);
#endif
