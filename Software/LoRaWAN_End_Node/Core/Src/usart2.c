/**
 * @file uart2_if.c
 * @brief UART2 Interface Driver Implementation
 * @author [Your Name]
 * @date [Current Date]
 */

#include "uart2_if.h"
#include <string.h>

/* Private types and structures */
/**
 * @brief Ring buffer structure for received data
 */
typedef struct {
    uint8_t buffer[UART2_RX_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t count;
    uint8_t temp_byte;  // Temporary storage for IT reception
} RingBuffer_t;

/* Private variables */
static RingBuffer_t rxRingBuffer;
static uint8_t txBuffer[UART2_TX_BUFFER_SIZE];
static volatile bool txBusy = false;
static volatile bool rxEnabled = false;
static UART2_Config_t currentConfig;

/* Callback function pointers */
static UART2_RxCallback_t rxCallback = NULL;
static UART2_TxCallback_t txCallback = NULL;
static UART2_ErrorCallback_t errorCallback = NULL;

/* External variables from HAL */
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;

/* Private function prototypes */
static void RingBuffer_Init(RingBuffer_t *rb);
static bool RingBuffer_Push(RingBuffer_t *rb, uint8_t data);
static bool RingBuffer_Pop(RingBuffer_t *rb, uint8_t *data);
static uint16_t RingBuffer_GetCount(RingBuffer_t *rb);
static void UART2_ErrorHandler(uint32_t error);

/* Function implementations */

UART2_Status_t UART2_Init(UART2_Config_t *config)
{
    if (config == NULL) {
        return UART2_INVALID_PARAM;
    }

    /* Store configuration */
    memcpy(&currentConfig, config, sizeof(UART2_Config_t));

    /* Initialize ring buffer */
    RingBuffer_Init(&rxRingBuffer);
    
    /* Configure UART */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = config->baudRate;
    huart2.Init.WordLength = config->wordLength;
    huart2.Init.StopBits = config->stopBits;
    huart2.Init.Parity = config->parity;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = config->hwFlowControl ? UART_HWCONTROL_RTS_CTS : UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    /* Initialize UART */
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        return UART2_ERROR;
    }

    /* Initialize DMA if requested */
    if (config->useDMA) {
        if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK) {
            return UART2_ERROR;
        }
    }

    return UART2_OK;
}

UART2_Status_t UART2_DeInit(void)
{
    /* Critical section start */
    __disable_irq();
    
    txBusy = false;
    rxEnabled = false;
    
    /* Abort any ongoing transfers */
    HAL_UART_AbortTransmit(&huart2);
    HAL_UART_AbortReceive(&huart2);
    
    /* Reset ring buffer */
    RingBuffer_Init(&rxRingBuffer);
    
    /* Critical section end */
    __enable_irq();
    
    /* DeInit UART */
    if (HAL_UART_DeInit(&huart2) != HAL_OK) {
        return UART2_ERROR;
    }

    return UART2_OK;
}

UART2_Status_t UART2_Transmit(uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (data == NULL || size == 0 || size > UART2_TX_BUFFER_SIZE) {
        return UART2_INVALID_PARAM;
    }

    /* Check if transmission is already ongoing */
    if (txBusy) {
        return UART2_BUSY;
    }

    txBusy = true;
    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart2, data, size, timeout);
    txBusy = false;

    return (status == HAL_OK) ? UART2_OK : UART2_ERROR;
}

UART2_Status_t UART2_TransmitDMA(uint8_t *data, uint16_t size)
{
    if (data == NULL || size == 0 || size > UART2_TX_BUFFER_SIZE) {
        return UART2_INVALID_PARAM;
    }

    if (txBusy || !currentConfig.useDMA) {
        return UART2_BUSY;
    }

    /* Copy data to TX buffer */
    __disable_irq();
    memcpy(txBuffer, data, size);
    txBusy = true;
    __enable_irq();
    
    if (HAL_UART_Transmit_DMA(&huart2, txBuffer, size) != HAL_OK) {
        txBusy = false;
        return UART2_ERROR;
    }

    return UART2_OK;
}

UART2_Status_t UART2_Receive(uint8_t *data, uint16_t size, uint32_t timeout)
{
    if (data == NULL || size == 0) {
        return UART2_INVALID_PARAM;
    }

    uint32_t tickstart = HAL_GetTick();
    uint16_t received = 0;
    uint8_t byte;

    while (received < size) {
        __disable_irq();
        bool success = RingBuffer_Pop(&rxRingBuffer, &byte);
        __enable_irq();

        if (success) {
            data[received++] = byte;
        }
        else if ((HAL_GetTick() - tickstart) > timeout) {
            return UART2_TIMEOUT;
        }
    }
    
    return UART2_OK;
}

UART2_Status_t UART2_ReceiveIT(void)
{
    if (rxEnabled) {
        return UART2_BUSY;
    }

    rxEnabled = true;
    if (HAL_UART_Receive_IT(&huart2, &rxRingBuffer.temp_byte, 1) != HAL_OK) {
        rxEnabled = false;
        return UART2_ERROR;
    }

    return UART2_OK;
}

uint16_t UART2_GetRxCount(void)
{
    __disable_irq();
    uint16_t count = RingBuffer_GetCount(&rxRingBuffer);
    __enable_irq();
    return count;
}

bool UART2_IsTxBusy(void)
{
    return txBusy;
}

void UART2_RegisterCallbacks(UART2_RxCallback_t rxCb, 
                           UART2_TxCallback_t txCb,
                           UART2_ErrorCallback_t errorCb)
{
    __disable_irq();
    rxCallback = rxCb;
    txCallback = txCb;
    errorCallback = errorCb;
    __enable_irq();
}

void UART2_AbortTransmit(void)
{
    HAL_UART_AbortTransmit(&huart2);
    txBusy = false;
}

void UART2_AbortReceive(void)
{
    HAL_UART_AbortReceive(&huart2);
    rxEnabled = false;
}

UART2_Status_t UART2_Flush(void)
{
    __disable_irq();
    RingBuffer_Init(&rxRingBuffer);
    __enable_irq();
    
    HAL_UART_AbortReceive(&huart2);
    HAL_UART_AbortTransmit(&huart2);
    txBusy = false;
    
    return UART2_ReceiveIT();
}

/* HAL Callback implementations */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        txBusy = false;
        if (txCallback != NULL) {
            txCallback();
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        /* Critical section for ring buffer operation */
        __disable_irq();
        
        if (!RingBuffer_Push(&rxRingBuffer, rxRingBuffer.temp_byte)) {
            /* Buffer full - remove oldest byte and add new one */
            uint8_t oldestByte;
            RingBuffer_Pop(&rxRingBuffer, &oldestByte);
            RingBuffer_Push(&rxRingBuffer, rxRingBuffer.temp_byte);
            
            if (errorCallback != NULL) {
                errorCallback(UART2_BUFFER_FULL);
            }
        }
        
        __enable_irq();

        /* Notify user if callback is registered */
        if (rxCallback != NULL) {
            rxCallback(&rxRingBuffer.temp_byte, 1);
        }

        /* Restart IT reception */
        if (HAL_UART_Receive_IT(&huart2, &rxRingBuffer.temp_byte, 1) != HAL_OK) {
            rxEnabled = false;
            if (errorCallback != NULL) {
                errorCallback(UART2_ERROR);
            }
        }
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        UART2_ErrorHandler(huart->ErrorCode);
    }
}

/* Private function implementations */
static void RingBuffer_Init(RingBuffer_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    rb->temp_byte = 0;
}

static bool RingBuffer_Push(RingBuffer_t *rb, uint8_t data)
{
    if (rb->count >= UART2_RX_BUFFER_SIZE) {
        return false;
    }

    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % UART2_RX_BUFFER_SIZE;
    rb->count++;
    return true;
}

static bool RingBuffer_Pop(RingBuffer_t *rb, uint8_t *data)
{
    if (rb->count == 0) {
        return false;
    }

    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % UART2_RX_BUFFER_SIZE;
    rb->count--;
    return true;
}

static uint16_t RingBuffer_GetCount(RingBuffer_t *rb)
{
    return rb->count;
}

static void UART2_ErrorHandler(uint32_t error)
{
    if (errorCallback != NULL) {
        errorCallback(error);
    }
    
    switch (error) {
        case HAL_UART_ERROR_PE:
            /* Parity error - reset reception */
            HAL_UART_AbortReceive(&huart2);
            UART2_ReceiveIT();
            break;
            
        case HAL_UART_ERROR_FE:
            /* Framing error - flush and restart */
            UART2_Flush();
            break;
            
        case HAL_UART_ERROR_ORE:
            /* Overrun error - flush and restart */
            UART2_Flush();
            break;
            
        case HAL_UART_ERROR_NE:
            /* Noise error - reset reception */
            HAL_UART_AbortReceive(&huart2);
            UART2_ReceiveIT();
            break;
            
        default:
            /* Serious error - full reset */
            UART2_DeInit();
            UART2_Init(&currentConfig);
            break;
    }
}