/*************************************************************************************
 Title   :   MAX6675 Cold-Junction-Compensated K-Type Thermocouple-to-Digital Converter Driver
 File    :   max6675.c
 Author  :   Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Date    :   2026-07-21
*************************************************************************************/
#include "max6675.h"

/*======================================================================================================
 =                                       Private Constants                                             =
 ======================================================================================================*/
#define MAX6675_OPEN_TC_BIT_MASK  (0x0004u)
#define MAX6675_DATA_SHIFT        (3u)
#define MAX6675_DATA_MASK         (0x0FFFu)
#define MAX6675_RESOLUTION_C      (0.25f)
#define MAX6675_FAHRENHEIT_SCALE  (1.8f)
#define MAX6675_FAHRENHEIT_OFFSET (32.0f)

/*======================================================================================================
 =                                       Private Variables                                             =
 ======================================================================================================*/
static MAX6675_Handle_t * s_activeHandle = NULL;

/*======================================================================================================
 =                                       Private Prototypes                                            =
 ======================================================================================================*/
static void MAX6675_CompleteRead(MAX6675_Handle_t * max6675Handle, MAX6675_Status_t status);

/*======================================================================================================
 =                                        Private Functions                                            =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX6675_CompleteRead
 | @brief   Deasserts chip-select and latches the outcome of a finished transfer.
 |
 | @param   max6675Handle Pointer to the handle whose transfer just finished
 | @param   status        MAX6675_STATUS_OK or MAX6675_STATUS_ERROR_SPI
 ---------------------------------------------------------------------------------------------------------*/
static void MAX6675_CompleteRead(MAX6675_Handle_t * max6675Handle, MAX6675_Status_t status)
{
    uint16_t rawWord;

    HAL_GPIO_WritePin(max6675Handle->csPort, max6675Handle->csPin, GPIO_PIN_SET);

    if (status == MAX6675_STATUS_OK)
    {
        rawWord = (uint16_t)(((uint16_t)max6675Handle->rxBuffer[0] << 8u) | (uint16_t)max6675Handle->rxBuffer[1]);

        if ((rawWord & MAX6675_OPEN_TC_BIT_MASK) != 0u)
        {
            status = MAX6675_STATUS_ERROR_OPEN_TC;
        }
        else
        {
            max6675Handle->lastTemperatureC = (float)((rawWord >> MAX6675_DATA_SHIFT) & MAX6675_DATA_MASK) * MAX6675_RESOLUTION_C;
        }
    }

    max6675Handle->lastStatus = status;
}

/*======================================================================================================
 =                                        Public Functions                                             =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX6675_Init
 | @brief   Binds a MAX6675 handle to its SPI peripheral, chip-select pin and transfer mode.
 |
 | @param   max6675Handle Pointer to the handle to initialize
 | @param   spiHandle     Pointer to the HAL SPI handle used for communication
 | @param   csPort        GPIO port of the chip-select pin
 | @param   csPin         GPIO pin of the chip-select pin
 | @param   timeoutMs     SPI transfer timeout in milliseconds, used only in MAX6675_MODE_POLLING
 | @param   mode          MAX6675_MODE_POLLING or MAX6675_MODE_DMA
 |
 | @return  MAX6675_STATUS_OK on success, MAX6675_STATUS_ERROR_NULL if a required pointer is NULL
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_Init(MAX6675_Handle_t * max6675Handle, SPI_HandleTypeDef * spiHandle, GPIO_TypeDef * csPort, uint16_t csPin, uint16_t timeoutMs, MAX6675_Mode_t mode)
{
    if ((max6675Handle == NULL) || (spiHandle == NULL) || (csPort == NULL))
    {
        return MAX6675_STATUS_ERROR_NULL;
    }

    max6675Handle->spiHandle        = spiHandle;
    max6675Handle->csPort           = csPort;
    max6675Handle->csPin            = csPin;
    max6675Handle->timeoutMs        = timeoutMs;
    max6675Handle->mode             = mode;
    max6675Handle->lastStatus       = MAX6675_STATUS_OK;
    max6675Handle->lastTemperatureC = 0.0f;

    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);

    s_activeHandle = max6675Handle;

    return MAX6675_STATUS_OK;
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX6675_StartRead
 | @brief   Starts a conversion-word transfer using the handle's configured mode.
 |
 | @param   max6675Handle Pointer to an initialized handle
 |
 | @return  In MAX6675_MODE_POLLING, the outcome of the completed transfer (MAX6675_STATUS_OK,
 |          MAX6675_STATUS_ERROR_SPI or MAX6675_STATUS_ERROR_OPEN_TC).
 |          In MAX6675_MODE_DMA, MAX6675_STATUS_OK if the DMA transfer was started (completion is
 |          reported later through MAX6675_GetReadStatus) or MAX6675_STATUS_ERROR_SPI if it could not
 |          be started.
 |          Either mode returns MAX6675_STATUS_ERROR_NULL if a required pointer is NULL, or
 |          MAX6675_STATUS_BUSY if a DMA transfer is already in flight.
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_StartRead(MAX6675_Handle_t * max6675Handle)
{
    if ((max6675Handle == NULL) || (max6675Handle->spiHandle == NULL) || (max6675Handle->csPort == NULL))
    {
        return MAX6675_STATUS_ERROR_NULL;
    }

    if (max6675Handle->lastStatus == MAX6675_STATUS_BUSY)
    {
        return MAX6675_STATUS_BUSY;
    }

    max6675Handle->lastStatus = MAX6675_STATUS_BUSY;

    HAL_GPIO_WritePin(max6675Handle->csPort, max6675Handle->csPin, GPIO_PIN_RESET);

    if (max6675Handle->mode == MAX6675_MODE_DMA)
    {
        if (HAL_SPI_Receive_DMA(max6675Handle->spiHandle, max6675Handle->rxBuffer, 2u) != HAL_OK)
        {
            MAX6675_CompleteRead(max6675Handle, MAX6675_STATUS_ERROR_SPI);
            return MAX6675_STATUS_ERROR_SPI;
        }

        return MAX6675_STATUS_OK;
    }

    if (HAL_SPI_Receive(max6675Handle->spiHandle, max6675Handle->rxBuffer, 2u, max6675Handle->timeoutMs) != HAL_OK)
    {
        MAX6675_CompleteRead(max6675Handle, MAX6675_STATUS_ERROR_SPI);
        return MAX6675_STATUS_ERROR_SPI;
    }

    MAX6675_CompleteRead(max6675Handle, MAX6675_STATUS_OK);

    return max6675Handle->lastStatus;
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX6675_GetReadStatus
 | @brief   Reports whether a transfer started by MAX6675_StartRead is still in flight.
 |
 | @param   max6675Handle Pointer to an initialized handle
 |
 | @return  MAX6675_STATUS_BUSY while a DMA transfer is in flight, MAX6675_STATUS_ERROR_NULL if
 |          max6675Handle is NULL, otherwise the latched outcome of the last completed transfer
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_GetReadStatus(const MAX6675_Handle_t * max6675Handle)
{
    if (max6675Handle == NULL)
    {
        return MAX6675_STATUS_ERROR_NULL;
    }

    return max6675Handle->lastStatus;
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX6675_GetTemperatureC
 | @brief   Retrieves the temperature in Celsius.
 |
 | @param   max6675Handle Pointer to an initialized handle
 | @param   temperatureC  Pointer to the variable receiving the temperature in Celsius
 |
 | @return  MAX6675_STATUS_OK on success,
 |          MAX6675_STATUS_ERROR_NULL if a required pointer is NULL,
 |          MAX6675_STATUS_BUSY if a transfer is still in flight,
 |          MAX6675_STATUS_ERROR_SPI/ERROR_OPEN_TC if the last transfer failed
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_GetTemperatureC(const MAX6675_Handle_t * max6675Handle, float * temperatureC)
{
    MAX6675_Status_t status;

    if (temperatureC == NULL)
    {
        return MAX6675_STATUS_ERROR_NULL;
    }

    status = MAX6675_GetReadStatus(max6675Handle);
    if (status != MAX6675_STATUS_OK)
    {
        return status;
    }

    *temperatureC = max6675Handle->lastTemperatureC;

    return MAX6675_STATUS_OK;
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX6675_GetTemperatureF
 | @brief   Retrieves the temperature in Fahrenheit.
 |
 | @param   max6675Handle Pointer to an initialized handle
 | @param   temperatureF  Pointer to the variable receiving the temperature in Fahrenheit
 |
 | @return  MAX6675_STATUS_OK on success,
 |          MAX6675_STATUS_ERROR_NULL if a required pointer is NULL,
 |          MAX6675_STATUS_BUSY if a transfer is still in flight,
 |          MAX6675_STATUS_ERROR_SPI/ERROR_OPEN_TC if the last transfer failed
 ---------------------------------------------------------------------------------------------------------*/
MAX6675_Status_t MAX6675_GetTemperatureF(const MAX6675_Handle_t * max6675Handle, float * temperatureF)
{
    MAX6675_Status_t status;
    float            temperatureC;

    if (temperatureF == NULL)
    {
        return MAX6675_STATUS_ERROR_NULL;
    }

    status = MAX6675_GetTemperatureC(max6675Handle, &temperatureC);
    if (status != MAX6675_STATUS_OK)
    {
        return status;
    }

    *temperatureF = (temperatureC * MAX6675_FAHRENHEIT_SCALE) + MAX6675_FAHRENHEIT_OFFSET;

    return MAX6675_STATUS_OK;
}

/*======================================================================================================
 =                                        HAL SPI Callbacks                                             =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @fn      HAL_SPI_RxCpltCallback
 | @brief   HAL DMA reception-complete callback. Completes the transfer on the active MAX6675 handle.
 |
 | @param   hspi Pointer to the SPI handle that completed its DMA transfer.
 ---------------------------------------------------------------------------------------------------------*/
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef * hspi)
{
    (void)hspi;

    if (s_activeHandle != NULL)
    {
        MAX6675_CompleteRead(s_activeHandle, MAX6675_STATUS_OK);
    }
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      HAL_SPI_ErrorCallback
 | @brief   HAL SPI error callback. Fails the transfer on the active MAX6675 handle so a pending DMA
 |          transfer is not left marked busy forever.
 |
 | @param   hspi Pointer to the SPI handle that reported the error.
 ---------------------------------------------------------------------------------------------------------*/
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef * hspi)
{
    (void)hspi;

    if (s_activeHandle != NULL)
    {
        MAX6675_CompleteRead(s_activeHandle, MAX6675_STATUS_ERROR_SPI);
    }
}
