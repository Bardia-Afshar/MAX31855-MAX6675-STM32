/*************************************************************************************
 Title   :   MAX6675 Cold-Junction-Compensated K-Type Thermocouple-to-Digital Converter Driver
 File    :   max6675.h
 Author  :   Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Date    :   2026-07-21
*************************************************************************************/
#ifndef MAX6675_H
#define MAX6675_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/*======================================================================================================
 =                                        Public Constants                                              =
 ======================================================================================================*/
#define MAX6675_CONVERSION_TIME_MS 				(220u)        // Max conversion time

/*======================================================================================================
 =                                          Public Types                                                =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @enum    MAX6675_Status_t
 ---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    MAX6675_STATUS_OK            = 0,
    MAX6675_STATUS_ERROR_NULL    = 1,
    MAX6675_STATUS_ERROR_SPI     = 2,
    MAX6675_STATUS_ERROR_OPEN_TC = 3,
    MAX6675_STATUS_BUSY          = 4
} MAX6675_Status_t;

/*--------------------------------------------------------------------------------------------------------
 | @enum    MAX6675_Mode_t
 | @brief   Selects how MAX6675_StartRead transfers the conversion word off the device.
 ---------------------------------------------------------------------------------------------------------*/
typedef enum
{
    MAX6675_MODE_POLLING = 0,  // MAX6675_StartRead blocks using HAL_SPI_Receive
    MAX6675_MODE_DMA     = 1   // MAX6675_StartRead returns immediately; HAL_SPI_Receive_DMA completes in the background
} MAX6675_Mode_t;

/*--------------------------------------------------------------------------------------------------------
 | @struct  MAX6675_Handle_t
 ---------------------------------------------------------------------------------------------------------*/
typedef struct
{
    SPI_HandleTypeDef       * spiHandle;
    GPIO_TypeDef            * csPort;
    uint16_t                  csPin;
    uint16_t                  timeoutMs;
    MAX6675_Mode_t            mode;
    uint8_t                   rxBuffer[2];
    volatile MAX6675_Status_t lastStatus;
    float                     lastTemperatureC;
} MAX6675_Handle_t;

/*======================================================================================================
 =                                        Public Functions                                             =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @brief   Binds a MAX6675 handle to its SPI peripheral, chip-select pin and transfer mode.
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_Init(MAX6675_Handle_t * max6675Handle, SPI_HandleTypeDef * spiHandle, GPIO_TypeDef * csPort, uint16_t csPin, uint16_t timeoutMs, MAX6675_Mode_t mode);

/*--------------------------------------------------------------------------------------------------------
 | @brief   Starts a conversion-word transfer using the handle's configured mode. In MAX6675_MODE_POLLING
 |          this blocks until the transfer finishes; in MAX6675_MODE_DMA it starts the DMA transfer and
 |          returns immediately, with completion reported through MAX6675_GetReadStatus.
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_StartRead(MAX6675_Handle_t * max6675Handle);

/*--------------------------------------------------------------------------------------------------------
 | @brief   Returns MAX6675_STATUS_BUSY while a transfer started by MAX6675_StartRead is still in flight
 |          (only possible in MAX6675_MODE_DMA), otherwise the latched outcome of the last transfer.
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_GetReadStatus(const MAX6675_Handle_t * max6675Handle);

/*--------------------------------------------------------------------------------------------------------
 | @brief   Retrieves the temperature in Celsius captured by the most recently completed transfer.
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_GetTemperatureC(const MAX6675_Handle_t * max6675Handle, float * temperatureC);

/*--------------------------------------------------------------------------------------------------------
 | @brief   Retrieves the temperature in Fahrenheit captured by the most recently completed transfer.
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_GetTemperatureF(const MAX6675_Handle_t * max6675Handle, float * temperatureF);

#ifdef __cplusplus
}
#endif

#endif /* MAX6675_H */
