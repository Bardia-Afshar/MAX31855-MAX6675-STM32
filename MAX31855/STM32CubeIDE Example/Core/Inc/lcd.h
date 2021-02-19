/*************************************************************************************
 Title	:   HD44780 LCD Library(4Bit) for STM32 Using HAL Libraries
 Author:    Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Software:  STM32CubeIDE
 Hardware:  Any STM32 device
*************************************************************************************/
#ifndef _LCD_H
#define _LCD_H
#include "main.h"
// ---------------------- LCD Configuration & PIN Definition -------------------------
#define LCD_ROWS         4             // Number of LCD ROWS
#define LCD_COLS         20            // Number of LCD Columns

#define LCD_RS_PORT      GPIOB        //------------------------------------------------------------------------------
#define LCD_RS_PIN       GPIO_PIN_1   //'                                                                            '
                                      //'         ---------------------------------------------------------          '
#define LCD_RW_PORT      GPIOA        //'         '                   Alphanumeric LCD                     '         '
#define LCD_RW_PIN       GPIO_PIN_9   //'         '                                                        '         '
                                      //'         '                    HD44780 Driver                      '         '
#define LCD_E_PORT       GPIOA        //'         '                                                        '         '
#define LCD_E_PIN        GPIO_PIN_10  //'         ----------------------------------------------------------         '
                                      //'                                                                            '
#define LCD_DATA4_PORT   GPIOA        //'                                                                            '
#define LCD_DATA4_PIN    GPIO_PIN_0   //------------------------------------------------------------------------------
                                      //||   ||   ||   ||   ||   ||   ||   ||   ||   ||   ||   ||   ||   ||   ||   ||
#define LCD_DATA5_PORT   GPIOA        //GND  VCC  VEE  RS   RW   EN   D0   D1   D2   D3   D4   D5   D6   D7   B+   B-
#define LCD_DATA5_PIN    GPIO_PIN_1

#define LCD_DATA6_PORT   GPIOA
#define LCD_DATA6_PIN    GPIO_PIN_2

#define LCD_DATA7_PORT   GPIOA
#define LCD_DATA7_PIN    GPIO_PIN_3
// ----------------------          Function Prototypes          -------------------------
void lcd_Init(void);                                  // 4Bit Mode - Display On - Cursor & Blink Off - Position 0,0
void lcd_DspOn(void);                                 // Turns Display On
void lcd_DspOff(void);                                // Turns Display Off
void lcd_CursorOn(void);                              // Turns Cursor On
void lcd_CursorOff(void);                             // Turns Cursor Off
void lcd_BlinkOn(void);                               // Cursor Blink On
void lcd_BlinkOff(void);                              // Cursor Blink Off
void lcd_gotoxy(uint8_t x,uint8_t y);                 // Moves Position to X,Y
void lcd_clrscr(void);                                // Clears Screen
void lcd_pos_puts(uint8_t x, uint8_t y, char* str);   // Writes on Position X,Y
void lcd_puts(char * str);                            // Write on Current X,Y
void lcd_Put(uint8_t Data);                           // Sends Data Directly to LCD
#endif
