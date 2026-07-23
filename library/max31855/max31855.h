/*************************************************************************************
 Title   :   MAX31855 Cold-Junction-Compensated K-Type Thermocouple-to-Digital Converter Driver
 File    :   max31855.h
 Author  :   Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Date    :   2026-07-23
*************************************************************************************/
#ifndef MAX31855_H
#define MAX31855_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================================================
 =                                        Public Constants                                              =
 ======================================================================================================*/
#define MAX31855_CONVERSION_TIME_MS 				(100u)        // Max conversion time

/*======================================================================================================
 =                                          Public Types                                                =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @enum    MAX31855_Status_t
 ---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    MAX31855_STATUS_OK              = 0,
    MAX31855_STATUS_ERROR_NULL      = 1,
    MAX31855_STATUS_ERROR_SPI       = 2,
    MAX31855_STATUS_ERROR_OPEN_TC   = 3,
    MAX31855_STATUS_ERROR_SHORT_GND = 4,
    MAX31855_STATUS_ERROR_SHORT_VCC = 5,
    MAX31855_STATUS_BUSY            = 6
} MAX31855_Status_t;

/*--------------------------------------------------------------------------------------------------------
 | @enum    MAX31855_Mode_t
 | @brief   Selects how MAX31855_StartRead transfers the conversion word off the device.
 ---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    MAX31855_MODE_POLLING = 0,  // MAX31855_StartRead blocks using HAL_SPI_Receive
    MAX31855_MODE_DMA     = 1   // MAX31855_StartRead returns immediately; HAL_SPI_Receive_DMA completes in the background
} MAX31855_Mode_t;

/*--------------------------------------------------------------------------------------------------------
 | @struct  MAX31855_Handle_t
 ---------------------------------------------------------------------------------------------------------*/
typedef struct
{
    SPI_HandleTypeDef         * spiHandle;
    GPIO_TypeDef              * csPort;
    uint16_t                    csPin;
    uint16_t                    timeoutMs;
    MAX31855_Mode_t             mode;
    uint8_t                     rxBuffer[4];
    volatile MAX31855_Status_t  lastStatus;
    float                       lastTemperatureC;
    float                       lastInternalTemperatureC;
} MAX31855_Handle_t;

/*======================================================================================================
 =                                        Public Functions                                             =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @brief   Binds a MAX31855 handle to its SPI peripheral, chip-select pin and transfer mode.
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_Init(MAX31855_Handle_t * max31855Handle, SPI_HandleTypeDef * spiHandle, GPIO_TypeDef * csPort, uint16_t csPin, uint16_t timeoutMs, MAX31855_Mode_t mode);

/*--------------------------------------------------------------------------------------------------------
 | @brief   Starts a conversion-word transfer using the handle's configured mode. In MAX31855_MODE_POLLING
 |          this blocks until the transfer finishes; in MAX31855_MODE_DMA it starts the DMA transfer and
 |          returns immediately, with completion reported through MAX31855_GetReadStatus.
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_StartRead(MAX31855_Handle_t * max31855Handle);

/*--------------------------------------------------------------------------------------------------------
 | @brief   Returns MAX31855_STATUS_BUSY while a transfer started by MAX31855_StartRead is still in flight
 |          (only possible in MAX31855_MODE_DMA), otherwise the latched outcome of the last transfer.
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_GetReadStatus(const MAX31855_Handle_t * max31855Handle);

/*--------------------------------------------------------------------------------------------------------
 | @brief   Retrieves the thermocouple (hot-junction) temperature in Celsius captured by the most
 |          recently completed transfer.
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_GetTemperatureC(const MAX31855_Handle_t * max31855Handle, float * temperatureC);

/*--------------------------------------------------------------------------------------------------------
 | @brief   Retrieves the thermocouple (hot-junction) temperature in Fahrenheit captured by the most
 |          recently completed transfer.
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_GetTemperatureF(const MAX31855_Handle_t * max31855Handle, float * temperatureF);

/*--------------------------------------------------------------------------------------------------------
 | @brief   Retrieves the cold-junction (on-chip reference) temperature in Celsius captured by the most
 |          recently completed transfer.
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_GetInternalTemperatureC(const MAX31855_Handle_t * max31855Handle, float * internalTemperatureC);

#ifdef __cplusplus
}
#endif

#endif /* MAX31855_H */
