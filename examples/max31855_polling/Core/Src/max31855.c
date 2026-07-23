/*************************************************************************************
 Title   :   MAX31855 Cold-Junction-Compensated K-Type Thermocouple-to-Digital Converter Driver
 File    :   max31855.c
 Author  :   Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
 Date    :   2026-07-23
*************************************************************************************/
#include "max31855.h"

/*======================================================================================================
 =                                       Private Constants                                             =
 ======================================================================================================*/
#define MAX31855_FAULT_BIT_MASK       (0x00010000u) // D16: set if any of OC/SCG/SCV is set
#define MAX31855_OC_BIT_MASK          (0x00000001u) // D0:  thermocouple open circuit
#define MAX31855_SCG_BIT_MASK         (0x00000002u) // D1:  thermocouple short-circuited to GND
#define MAX31855_SCV_BIT_MASK         (0x00000004u) // D2:  thermocouple short-circuited to VCC

#define MAX31855_TC_DATA_SHIFT        (18u)         // D31:D18, signed thermocouple temperature
#define MAX31855_TC_DATA_MASK         (0x3FFFu)
#define MAX31855_TC_SIGN_BIT          (0x2000u)
#define MAX31855_TC_RESOLUTION_C      (0.25f)

#define MAX31855_INTERNAL_DATA_SHIFT  (4u)          // D15:D4, signed cold-junction temperature
#define MAX31855_INTERNAL_DATA_MASK   (0x0FFFu)
#define MAX31855_INTERNAL_SIGN_BIT    (0x0800u)
#define MAX31855_INTERNAL_RESOLUTION_C (0.0625f)

#define MAX31855_FAHRENHEIT_SCALE     (1.8f)
#define MAX31855_FAHRENHEIT_OFFSET    (32.0f)

/*======================================================================================================
 =                                       Private Variables                                             =
 ======================================================================================================*/
static MAX31855_Handle_t * s_activeHandle = NULL;

/*======================================================================================================
 =                                       Private Prototypes                                            =
 ======================================================================================================*/
static void MAX31855_CompleteRead(MAX31855_Handle_t * max31855Handle, MAX31855_Status_t status);

/*======================================================================================================
 =                                        Private Functions                                            =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX31855_CompleteRead
 | @brief   Deasserts chip-select and latches the outcome of a finished transfer, parsing rxBuffer into
 |          the thermocouple and cold-junction temperatures when the transfer itself succeeded, or the
 |          specific fault reported by the device otherwise.
 |
 | @param   max31855Handle Pointer to the handle whose transfer just finished
 | @param   status         Outcome to latch: MAX31855_STATUS_OK or MAX31855_STATUS_ERROR_SPI
 ---------------------------------------------------------------------------------------------------------*/
static void MAX31855_CompleteRead(MAX31855_Handle_t * max31855Handle, MAX31855_Status_t status)
{
    uint32_t rawWord;
    int32_t  rawTc;
    int32_t  rawInternal;

    HAL_GPIO_WritePin(max31855Handle->csPort, max31855Handle->csPin, GPIO_PIN_SET);

    if (status == MAX31855_STATUS_OK)
    {
        rawWord = (((uint32_t)max31855Handle->rxBuffer[0] << 24u) |
                   ((uint32_t)max31855Handle->rxBuffer[1] << 16u) |
                   ((uint32_t)max31855Handle->rxBuffer[2] << 8u)  |
                   ((uint32_t)max31855Handle->rxBuffer[3]));

        if ((rawWord & MAX31855_FAULT_BIT_MASK) != 0u)
        {
            if ((rawWord & MAX31855_OC_BIT_MASK) != 0u)
            {
                status = MAX31855_STATUS_ERROR_OPEN_TC;
            }
            else if ((rawWord & MAX31855_SCG_BIT_MASK) != 0u)
            {
                status = MAX31855_STATUS_ERROR_SHORT_GND;
            }
            else
            {
                status = MAX31855_STATUS_ERROR_SHORT_VCC;
            }
        }
        else
        {
            rawTc = (int32_t)((rawWord >> MAX31855_TC_DATA_SHIFT) & MAX31855_TC_DATA_MASK);
            if ((rawTc & MAX31855_TC_SIGN_BIT) != 0)
            {
                rawTc -= (int32_t)(MAX31855_TC_DATA_MASK + 1u);
            }
            max31855Handle->lastTemperatureC = (float)rawTc * MAX31855_TC_RESOLUTION_C;

            rawInternal = (int32_t)((rawWord >> MAX31855_INTERNAL_DATA_SHIFT) & MAX31855_INTERNAL_DATA_MASK);
            if ((rawInternal & MAX31855_INTERNAL_SIGN_BIT) != 0)
            {
                rawInternal -= (int32_t)(MAX31855_INTERNAL_DATA_MASK + 1u);
            }
            max31855Handle->lastInternalTemperatureC = (float)rawInternal * MAX31855_INTERNAL_RESOLUTION_C;
        }
    }

    max31855Handle->lastStatus = status;
}

/*======================================================================================================
 =                                        Public Functions                                             =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX31855_Init
 | @brief   Binds a MAX31855 handle to its SPI peripheral, chip-select pin and transfer mode.
 |
 | @param   max31855Handle Pointer to the handle to initialize
 | @param   spiHandle      Pointer to the HAL SPI handle used for communication
 | @param   csPort         GPIO port of the chip-select pin
 | @param   csPin          GPIO pin of the chip-select pin
 | @param   timeoutMs      SPI transfer timeout in milliseconds, used only in MAX31855_MODE_POLLING
 | @param   mode           MAX31855_MODE_POLLING or MAX31855_MODE_DMA
 |
 | @return  MAX31855_STATUS_OK on success, MAX31855_STATUS_ERROR_NULL if a required pointer is NULL
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_Init(MAX31855_Handle_t * max31855Handle, SPI_HandleTypeDef * spiHandle, GPIO_TypeDef * csPort, uint16_t csPin, uint16_t timeoutMs, MAX31855_Mode_t mode)
{
    if ((max31855Handle == NULL) || (spiHandle == NULL) || (csPort == NULL))
    {
        return MAX31855_STATUS_ERROR_NULL;
    }

    max31855Handle->spiHandle                = spiHandle;
    max31855Handle->csPort                   = csPort;
    max31855Handle->csPin                    = csPin;
    max31855Handle->timeoutMs                = timeoutMs;
    max31855Handle->mode                     = mode;
    max31855Handle->lastStatus               = MAX31855_STATUS_OK;
    max31855Handle->lastTemperatureC         = 0.0f;
    max31855Handle->lastInternalTemperatureC = 0.0f;

    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);

    s_activeHandle = max31855Handle;

    return MAX31855_STATUS_OK;
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX31855_StartRead
 | @brief   Starts a conversion-word transfer using the handle's configured mode.
 |
 | @param   max31855Handle Pointer to an initialized handle
 |
 | @return  In MAX31855_MODE_POLLING, the outcome of the completed transfer (MAX31855_STATUS_OK,
 |          MAX31855_STATUS_ERROR_SPI, MAX31855_STATUS_ERROR_OPEN_TC, MAX31855_STATUS_ERROR_SHORT_GND or
 |          MAX31855_STATUS_ERROR_SHORT_VCC).
 |          In MAX31855_MODE_DMA, MAX31855_STATUS_OK if the DMA transfer was started (completion is
 |          reported later through MAX31855_GetReadStatus) or MAX31855_STATUS_ERROR_SPI if it could not
 |          be started.
 |          Either mode returns MAX31855_STATUS_ERROR_NULL if a required pointer is NULL, or
 |          MAX31855_STATUS_BUSY if a DMA transfer is already in flight.
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_StartRead(MAX31855_Handle_t * max31855Handle)
{
    if ((max31855Handle == NULL) || (max31855Handle->spiHandle == NULL) || (max31855Handle->csPort == NULL))
    {
        return MAX31855_STATUS_ERROR_NULL;
    }

    if (max31855Handle->lastStatus == MAX31855_STATUS_BUSY)
    {
        return MAX31855_STATUS_BUSY;
    }

    max31855Handle->lastStatus = MAX31855_STATUS_BUSY;

    HAL_GPIO_WritePin(max31855Handle->csPort, max31855Handle->csPin, GPIO_PIN_RESET);

    if (max31855Handle->mode == MAX31855_MODE_DMA)
    {
        if (HAL_SPI_Receive_DMA(max31855Handle->spiHandle, max31855Handle->rxBuffer, 4u) != HAL_OK)
        {
            MAX31855_CompleteRead(max31855Handle, MAX31855_STATUS_ERROR_SPI);
            return MAX31855_STATUS_ERROR_SPI;
        }

        return MAX31855_STATUS_OK;
    }

    if (HAL_SPI_Receive(max31855Handle->spiHandle, max31855Handle->rxBuffer, 4u, max31855Handle->timeoutMs) != HAL_OK)
    {
        MAX31855_CompleteRead(max31855Handle, MAX31855_STATUS_ERROR_SPI);
        return MAX31855_STATUS_ERROR_SPI;
    }

    MAX31855_CompleteRead(max31855Handle, MAX31855_STATUS_OK);

    return max31855Handle->lastStatus;
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX31855_GetReadStatus
 | @brief   Reports whether a transfer started by MAX31855_StartRead is still in flight.
 |
 | @param   max31855Handle Pointer to an initialized handle
 |
 | @return  MAX31855_STATUS_BUSY while a DMA transfer is in flight, MAX31855_STATUS_ERROR_NULL if
 |          max31855Handle is NULL, otherwise the latched outcome of the last completed transfer
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_GetReadStatus(const MAX31855_Handle_t * max31855Handle)
{
    if (max31855Handle == NULL)
    {
        return MAX31855_STATUS_ERROR_NULL;
    }

    return max31855Handle->lastStatus;
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX31855_GetTemperatureC
 | @brief   Retrieves the thermocouple (hot-junction) temperature in Celsius captured by the most
 |          recently completed transfer.
 |
 | @param   max31855Handle Pointer to an initialized handle
 | @param   temperatureC   Pointer to the variable receiving the temperature in Celsius
 |
 | @return  MAX31855_STATUS_OK on success,
 |          MAX31855_STATUS_ERROR_NULL if a required pointer is NULL,
 |          MAX31855_STATUS_BUSY if a transfer is still in flight,
 |          MAX31855_STATUS_ERROR_SPI/ERROR_OPEN_TC/ERROR_SHORT_GND/ERROR_SHORT_VCC if the last transfer failed
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_GetTemperatureC(const MAX31855_Handle_t * max31855Handle, float * temperatureC)
{
    MAX31855_Status_t status;

    if (temperatureC == NULL)
    {
        return MAX31855_STATUS_ERROR_NULL;
    }

    status = MAX31855_GetReadStatus(max31855Handle);
    if (status != MAX31855_STATUS_OK)
    {
        return status;
    }

    *temperatureC = max31855Handle->lastTemperatureC;

    return MAX31855_STATUS_OK;
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX31855_GetTemperatureF
 | @brief   Retrieves the thermocouple (hot-junction) temperature in Fahrenheit captured by the most
 |          recently completed transfer.
 |
 | @param   max31855Handle Pointer to an initialized handle
 | @param   temperatureF   Pointer to the variable receiving the temperature in Fahrenheit
 |
 | @return  MAX31855_STATUS_OK on success,
 |          MAX31855_STATUS_ERROR_NULL if a required pointer is NULL,
 |          MAX31855_STATUS_BUSY if a transfer is still in flight,
 |          MAX31855_STATUS_ERROR_SPI/ERROR_OPEN_TC/ERROR_SHORT_GND/ERROR_SHORT_VCC if the last transfer failed
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_GetTemperatureF(const MAX31855_Handle_t * max31855Handle, float * temperatureF)
{
    MAX31855_Status_t status;
    float              temperatureC;

    if (temperatureF == NULL)
    {
        return MAX31855_STATUS_ERROR_NULL;
    }

    status = MAX31855_GetTemperatureC(max31855Handle, &temperatureC);
    if (status != MAX31855_STATUS_OK)
    {
        return status;
    }

    *temperatureF = (temperatureC * MAX31855_FAHRENHEIT_SCALE) + MAX31855_FAHRENHEIT_OFFSET;

    return MAX31855_STATUS_OK;
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      MAX31855_GetInternalTemperatureC
 | @brief   Retrieves the cold-junction (on-chip reference) temperature in Celsius captured by the most
 |          recently completed transfer.
 |
 | @param   max31855Handle         Pointer to an initialized handle
 | @param   internalTemperatureC   Pointer to the variable receiving the temperature in Celsius
 |
 | @return  MAX31855_STATUS_OK on success,
 |          MAX31855_STATUS_ERROR_NULL if a required pointer is NULL,
 |          MAX31855_STATUS_BUSY if a transfer is still in flight,
 |          MAX31855_STATUS_ERROR_SPI/ERROR_OPEN_TC/ERROR_SHORT_GND/ERROR_SHORT_VCC if the last transfer failed
 ---------------------------------------------------------------------------------------------------------*/
MAX31855_Status_t MAX31855_GetInternalTemperatureC(const MAX31855_Handle_t * max31855Handle, float * internalTemperatureC)
{
    MAX31855_Status_t status;

    if (internalTemperatureC == NULL)
    {
        return MAX31855_STATUS_ERROR_NULL;
    }

    status = MAX31855_GetReadStatus(max31855Handle);
    if (status != MAX31855_STATUS_OK)
    {
        return status;
    }

    *internalTemperatureC = max31855Handle->lastInternalTemperatureC;

    return MAX31855_STATUS_OK;
}

/*======================================================================================================
 =                                        HAL SPI Callbacks                                             =
 ======================================================================================================*/

/*--------------------------------------------------------------------------------------------------------
 | @fn      HAL_SPI_RxCpltCallback
 | @brief   HAL DMA reception-complete callback. Completes the transfer on the active MAX31855 handle.
 |
 | @param   hspi Pointer to the SPI handle that completed its DMA transfer.
 ---------------------------------------------------------------------------------------------------------*/
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef * hspi)
{
    (void)hspi;

    if (s_activeHandle != NULL)
    {
        MAX31855_CompleteRead(s_activeHandle, MAX31855_STATUS_OK);
    }
}

/*--------------------------------------------------------------------------------------------------------
 | @fn      HAL_SPI_ErrorCallback
 | @brief   HAL SPI error callback. Fails the transfer on the active MAX31855 handle so a pending DMA
 |          transfer is not left marked busy forever.
 |
 | @param   hspi Pointer to the SPI handle that reported the error.
 ---------------------------------------------------------------------------------------------------------*/
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef * hspi)
{
    (void)hspi;

    if (s_activeHandle != NULL)
    {
        MAX31855_CompleteRead(s_activeHandle, MAX31855_STATUS_ERROR_SPI);
    }
}
