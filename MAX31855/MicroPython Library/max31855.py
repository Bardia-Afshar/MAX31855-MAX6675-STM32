######################################################################################
#Title   :  MAXIM Integrated MAX31855 Library for Raspberry pi Pico
#Author  :  Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
#Language:  Python
#Hardware:  Raspberry pi pico
#####################################################################################
import ustruct
from machine import Pin, SPI


class MAX31855:
    def __init__(self,SPInum=0,CS=1,MISO=4,SCK=6):
        self.spi = SPI(SPInum, 100_000, polarity=0, phase=0,miso=Pin(MISO),sck=Pin(SCK))
        self.cs =  Pin(CS, Pin.OUT)    #Chip Select Pin
        self.data = bytearray(4)

    def ReadTemp(self):
        self.cs.value(0)             # ChipSelect -> Low to Start SPI
        self.spi.readinto(self.data) # Data Read
        self.cs.value(1)             # ChipSelect -> High to Stop SPI
        error=self.data[3]&0x07      #Thermocouple Error Detection
        if(error):                   # Returns error number and Temp=0              
            Temp=0
            return error, Temp
        sign=(self.data[0]&(0x80))>>7  # Detecting positive or negative temperature
        if sign==1: #Calculation for negative temperature
            Temp = (self.data[0] << 6) | (self.data[1] >> 2)
            Temp&=0b01111111111111
            Temp^=0b01111111111111
            Temp=-Temp*0.25
        else: #Calculation for positive temperature
            Temp = (self.data[0] << 6) | (self.data[1] >> 2)
            Temp=Temp *0.25
        return error, Temp        
  

