/*************************************************************************************
 Title	 :  MAXIM Integrated MAX6675 Library for STM32 Using HAL Libraries
 Author  :  Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Software:  STM32CubeIDE
 Hardware:  Any STM32 device
*************************************************************************************/
#ifndef INC_MAX6675_H_
#define INC_MAX6675_H_
#include "main.h"

// ------------------------- Defines -------------------------
#define SSPORT GPIOB       // GPIO Port of Chip Select(Slave Select)
#define SSPIN  GPIO_PIN_1  // GPIO PIN of Chip Select(Slave Select)
// ------------------------- Functions  ----------------------
float Max6675_Read_Temp(void);
#endif
