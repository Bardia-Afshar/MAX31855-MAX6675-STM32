/*************************************************************************************
 Title	:   HD44780 LCD Library(4Bit) for STM32 Using HAL Libraries
 Author:    Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Software:  STM32CubeIDE
 Hardware:  Any STM32 device
*************************************************************************************/
#include "LCD.h"
// -------------------------- X , Y Position Variable -------------------------------
uint8_t Xcurrent=0;                                  // X Position
uint8_t Ycurrent=0;                                  // Y Position
// ------------------------ Command & Data Write functions --------------------------
static void Command(uint8_t cmd);                   // 8Bit Data Transfer in command Mode
static void Command4Bit(uint8_t cmd);               // 2*4Bit Data Transfer in command Mode
static void lcd_write_data(uint8_t data);           // Data transfer in Non-command Mode
void Delay_us(uint32_t us)
{
	us=us*50;
	for(uint32_t i=0;i<us;i++)
	{
		__ASM("NOP");
	}
}
// ------------------------------ LCD Pins Status -----------------------------------
#define LCD_RS_LOW              HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN,GPIO_PIN_RESET)
#define LCD_RS_HIGH             HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN,GPIO_PIN_SET)
#define LCD_E_LOW               HAL_GPIO_WritePin(LCD_E_PORT,  LCD_E_PIN,GPIO_PIN_RESET)
#define LCD_E_HIGH              HAL_GPIO_WritePin(LCD_E_PORT,  LCD_E_PIN,GPIO_PIN_SET)
#define LCD_E_BLINK             LCD_E_HIGH; Delay_us(15); LCD_E_LOW; Delay_us(15)
// ------------------------------ Command Addresses ----------------------------------
#define LCD_CLR           0x01                       // Screen Clear Command Address
#define LCD_DISP_ON       0x0C                       // Screen On Command Address
#define LCD_DISP_OFF      0x08                       // Screen Off Command Address
#define LCD_BLINK_ON      0x0D                       // Cursor Blink On Command Address
#define LCD_BLINK_OFF     0x0C                       // Cursor Blink Off Command Address
#define LCD_CURSOR_ON     0x0E                       // Cursor On Command Address
#define LCD_CURSOR_OFF    0x0C                       // Cursor Off Command Address
#define LCD_2LINE         0x08                       // Address for More Than 2 Line LCDs (For Example 4*20)
// ------------------------- Initializing Function for LCD ----------------------------
void lcd_Init(void)
{	
	uint8_t FuncCnTrL=0;                             // Tuning Variable
    HAL_Delay(1);
	lcd_gotoxy(0,0);                                 // Sets Cursor on 0,0
    if (LCD_ROWS > 1){FuncCnTrL|=LCD_2LINE;}         // Changing Address if --> LCD Rows>2 Line
    Command4Bit(0x03);                               // Starts Commands to set LCD in 4Bit Interface
	HAL_Delay(5);
	Command4Bit(0x03);
	HAL_Delay(5);
	Command4Bit(0x03);
	HAL_Delay(5);
	Command4Bit(0x02);
	HAL_Delay(5);
// ----------- Turns Displays on - No Cursor - No Blinking - Position 0,0 - Default Font
	Command(0x20 | FuncCnTrL);
    Command(0x08|0x04);
    lcd_clrscr();
	Command(0x04 | 0x02);
	HAL_Delay(5);
}
// --------------------------------- Screen Clear Function -------------------------------
void lcd_clrscr(void)
{
	Command(LCD_CLR);
	HAL_Delay(1);
}
// ----------------------------------- Puts String on LCD ---------------------------------
void lcd_puts(char * str){
while (*str)
	  {
			if (Xcurrent >= LCD_COLS)
	    {
				Xcurrent = 0;
				Ycurrent=0;
				lcd_gotoxy(Xcurrent, Ycurrent);
			}
			if (*str == '\n')
	    {
				Ycurrent++;
				lcd_gotoxy(Xcurrent, Ycurrent);
			}
	    else if (*str == '\r')
	    {
	    	lcd_gotoxy(0, Ycurrent);
			}
	    else
	    {
				lcd_write_data(*str);
				Xcurrent++;
			}
			str++;
		}
}
// --------------------------------- Puts String on X,Y Position ---------------------------------
void lcd_pos_puts(uint8_t x, uint8_t y, char* str)
{
lcd_gotoxy(x, y);
lcd_puts(str);
}
//---------------------------------------- Display On Function ---------------------------------------
void lcd_DspOn(void)
{
Command(LCD_DISP_ON);
}
//---------------------------------------- Display Off Function --------------------------------------
void lcd_DspOff(void)
{
Command(LCD_DISP_OFF);
}
//---------------------------------------- Blink On Function -----------------------------------------
void lcd_BlinkOn(void)
{
Command(LCD_BLINK_ON);
}
//---------------------------------------- Blink Off Function -----------------------------------------
void lcd_BlinkOff(void)
{
Command(LCD_BLINK_OFF);
}
//---------------------------------------- Cursor On Function -----------------------------------------
void lcd_CursorOn(void)
{
Command(LCD_CURSOR_ON);
}
//---------------------------------------- Cursor Off Function ----------------------------------------
void lcd_CursorOff(void)
{
Command(LCD_CURSOR_OFF);
}
// ----------------------------------- Command Data Transfer Function ---------------------------------
static void Command(uint8_t cmd)
{
LCD_RS_LOW;
Command4Bit(cmd >> 4);
Command4Bit(cmd & 0x0F);
}
// ----------------------------------------- Sends RAW DATA to LCD ------------------------------------
static void lcd_write_data(uint8_t data)
{
LCD_RS_HIGH;
Command4Bit(data >> 4);
Command4Bit(data & 0x0F);
}
// -------------------------------- Command Data Transfer Function(4Bit Mode) -------------------------
static void Command4Bit(uint8_t cmd)
{
HAL_GPIO_WritePin(LCD_DATA7_PORT, LCD_DATA7_PIN, (GPIO_PinState)(cmd & 0x08));
HAL_GPIO_WritePin(LCD_DATA6_PORT, LCD_DATA6_PIN, (GPIO_PinState)(cmd & 0x04));
HAL_GPIO_WritePin(LCD_DATA5_PORT, LCD_DATA5_PIN, (GPIO_PinState)(cmd & 0x02));
HAL_GPIO_WritePin(LCD_DATA4_PORT, LCD_DATA4_PIN, (GPIO_PinState)(cmd & 0x01));
LCD_E_BLINK;
}
// ------------------------------------ Cursor Position set Function ----------------------------------
void lcd_gotoxy(uint8_t x, uint8_t y)
{
uint8_t row_addr[] = {0x00, 0x40, 0x14, 0x54}; // Addresses for First Character on each line
if (y >= LCD_ROWS)
y = 0;
Xcurrent = x;
Ycurrent = y;
Command(0x80 | (x + row_addr[y]));
}
// ------------------------ Puts RAW DATA on LCD ( For custom Characters,etc. ) ------------------------
void lcd_Put(uint8_t Data)
{
lcd_write_data(Data);
}

