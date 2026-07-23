#*************************************************************************************
# Title   :   MicroPython driver for the MAXIM Integrated MAX31855
#             cold-junction compensated thermocouple-to-digital converter
# File    :   max31855.py
# Author  :   Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
# Date    :   2026-07-19
#*************************************************************************************

from machine import Pin, SPI

#======================================================================================================
#=                                   MAX31855 field masks                                              =
#======================================================================================================
# Byte 3 of the 32-bit SPI frame, D[3:0].

MAX31855_FAULT_MASK = 0x07   # Any thermocouple fault (OC | SCG | SCV)
MAX31855_FAULT_OC   = 0x01   # Open circuit: thermocouple has no connection
MAX31855_FAULT_SCG  = 0x02   # Thermocouple short-circuited to GND
MAX31855_FAULT_SCV  = 0x04   # Thermocouple short-circuited to VCC

# Resolution of the 14-bit thermocouple temperature field, in degrees Celsius per LSB.
MAX31855_TEMP_LSB = 0.25

# Bit width of the signed thermocouple temperature field (D[31:18]).
MAX31855_TEMP_BITS = 14


#======================================================================================================
#=                                            MAX31855                                                 =
#======================================================================================================

class MAX31855:

    #--------------------------------------------------------------------------------------------------------
    # | @fn      __init__
    # | @brief   Construct the driver and initialize the SPI bus and chip-select pin.
    # |
    # | @param   spi_id SPI peripheral ID (0 or 1 on the Pico).
    # | @param   cs     GPIO pin number used as chip select.
    # | @param   miso   GPIO pin number used as SPI MISO (data out from the MAX31855).
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
        # Chip-select pin, held high (deselected) except during a read.
        self.cs = Pin(cs, Pin.OUT, value=1)
        # Reusable 4-byte receive buffer for the 32-bit SPI frame.
        self._data = bytearray(4)

    #--------------------------------------------------------------------------------------------------------
    # | @fn      ReadTemp
    # | @brief   Read and decode a single temperature conversion from the sensor.
    # |
    # | @return  Tuple of (fault, temperature_c):
    # |          - fault: 0 if no fault, otherwise a bitmask of MAX31855_FAULT_OC,
    # |            MAX31855_FAULT_SCG and/or MAX31855_FAULT_SCV.
    # |          - temperature_c: thermocouple temperature in degrees Celsius,
    # |            or 0 if a fault was detected.
    #---------------------------------------------------------------------------------------------------------
    def ReadTemp(self):
        self.cs.value(0)                                   # Select the chip to begin the SPI transfer.
        self.spi.readinto(self._data)                      # Clock in the 32-bit conversion frame.
        self.cs.value(1)                                   # Deselect the chip.

        fault = self._data[3] & MAX31855_FAULT_MASK
        if fault:
            return fault, 0

        raw = (self._data[0] << 6) | (self._data[1] >> 2)  # 14-bit signed field.
        if raw & (1 << (MAX31855_TEMP_BITS - 1)):          # Sign bit set: negative temperature.
            raw -= 1 << MAX31855_TEMP_BITS                 # Two's-complement conversion to signed int.

        temperature_c = raw * MAX31855_TEMP_LSB
        return fault, temperature_c
