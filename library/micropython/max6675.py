#*************************************************************************************
# Title   :   MicroPython driver for the MAXIM Integrated MAX6675
#             cold-junction compensated thermocouple-to-digital converter
# File    :   max6675.py
# Author  :   Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
# Date    :   2026-07-19
#*************************************************************************************

from machine import Pin, SPI

#======================================================================================================
#=                                    MAX6675 field masks                                              =
#======================================================================================================

MAX6675_FAULT_BIT = 0x0004   # D2: thermocouple input open (only fault the MAX6675 reports)
MAX6675_TEMP_MASK = 0x0FFF   # D[14:3]: 12-bit unsigned temperature field

# Resolution of the 12-bit thermocouple temperature field, in degrees Celsius per LSB.
MAX6675_TEMP_LSB = 0.25


#======================================================================================================
#=                                            MAX6675                                                  =
#======================================================================================================

class MAX6675:

    #--------------------------------------------------------------------------------------------------------
    # | @fn      __init__
    # | @brief   Construct the driver and initialize the SPI bus and chip-select pin.
    # |
    # | @param   spi_id SPI peripheral ID (0 or 1 on the Pico).
    # | @param   cs     GPIO pin number used as chip select.
    # | @param   miso   GPIO pin number used as SPI MISO (data out from the MAX6675).
    # | @param   sck    GPIO pin number used as SPI clock.
    # |
    # | @return  None
    #---------------------------------------------------------------------------------------------------------
    def __init__(self, spi_id=0, cs=1, miso=4, sck=6):

        self.spi = SPI(
            spi_id,
            baudrate=100_000,
            polarity=0,
            phase=0,
            miso=Pin(miso),
            sck=Pin(sck),
        )
     
        self.cs = Pin(cs, Pin.OUT, value=1)
        
        self._data = bytearray(2)

    #--------------------------------------------------------------------------------------------------------
    # | @fn      ReadTemp
    # | @brief   Read and decode a single temperature conversion from the sensor.
    # |
    # | @return  Tuple of (fault, temperature_c):
    # |          - fault: 0 if no fault, otherwise MAX6675_FAULT_BIT (thermocouple
    # |            input open).
    # |          - temperature_c: thermocouple temperature in degrees Celsius,
    # |            or 0 if a fault was detected.
    #---------------------------------------------------------------------------------------------------------
    def ReadTemp(self):
        self.cs.value(0)                           # Select the chip to begin the SPI transfer.
        self.spi.readinto(self._data)              # Clock in the 16-bit conversion frame.
        self.cs.value(1)                           # Deselect the chip.

        raw = (self._data[0] << 8) | self._data[1]

        fault = raw & MAX6675_FAULT_BIT            # D2 set: thermocouple input open.
        if fault:
            return fault, 0

        temp_raw = (raw >> 3) & MAX6675_TEMP_MASK  # 12-bit unsigned field, D[14:3].
        temperature_c = temp_raw * MAX6675_TEMP_LSB
        return fault, temperature_c
