/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART2_H__
#define __USART2_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

/* Configuration Parameters */
#define UART2_RX_BUFFER_SIZE    256     /**< Size of receive ring buffer */
#define UART2_TX_BUFFER_SIZE    256     /**< Size of transmit buffer */
#define UART2_TIMEOUT_MS        1000    /**< Default timeout in milliseconds */

/**
 * @brief UART2 Status codes
 */
typedef enum {
    UART2_OK = 0,           /**< Operation completed successfully */
    UART2_ERROR,            /**< General error occurred */
    UART2_BUSY,             /**< UART2 is busy with another operation */
    UART2_TIMEOUT,          /**< Operation timed out */
    UART2_BUFFER_FULL,      /**< Buffer is full */
    UART2_BUFFER_EMPTY,     /**< Buffer is empty */
    UART2_INVALID_PARAM     /**< Invalid parameter provided */
} UART2_Status_t;

/**
 * @brief UART2 Configuration structure
 */
typedef struct {
    uint32_t baudRate;      /**< Baud rate setting */
    uint32_t wordLength;    /**< Word length setting */
    uint32_t stopBits;      /**< Stop bits setting */
    uint32_t parity;        /**< Parity setting */
    bool hwFlowControl;     /**< Hardware flow control enable/disable */
    bool useDMA;           /**< DMA usage enable/disable */
} UART2_Config_t;

/**
 * @brief Callback function types
 */
typedef void (*UART2_RxCallback_t)(uint8_t *data, uint16_t size);
typedef void (*UART2_TxCallback_t)(void);
typedef void (*UART2_ErrorCallback_t)(uint32_t errorCode);

/* Function Prototypes */

/**
 * @brief Initialize UART2 with given configuration
 * @param config Pointer to configuration structure
 * @return UART2_Status_t Status of operation
 */
UART2_Status_t UART2_Init(UART2_Config_t *config);

/**
 * @brief De-initialize UART2
 * @return UART2_Status_t Status of operation
 */
UART2_Status_t UART2_DeInit(void);

/**
 * @brief Transmit data using polling method
 * @param data Pointer to data buffer
 * @param size Size of data to transmit
 * @param timeout Timeout value in milliseconds
 * @return UART2_Status_t Status of operation
 */
UART2_Status_t UART2_Transmit(uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief Transmit data using DMA
 * @param data Pointer to data buffer
 * @param size Size of data to transmit
 * @return UART2_Status_t Status of operation
 */
UART2_Status_t UART2_TransmitDMA(uint8_t *data, uint16_t size);

/**
 * @brief Receive data using polling method
 * @param data Pointer to data buffer
 * @param size Size of data to receive
 * @param timeout Timeout value in milliseconds
 * @return UART2_Status_t Status of operation
 */
UART2_Status_t UART2_Receive(uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * @brief Start receive in interrupt mode
 * @return UART2_Status_t Status of operation
 */
UART2_Status_t UART2_ReceiveIT(void);

/**
 * @brief Get number of bytes in receive buffer
 * @return uint16_t Number of bytes available
 */
uint16_t UART2_GetRxCount(void);

/**
 * @brief Check if transmission is ongoing
 * @return bool true if busy, false if idle
 */
bool UART2_IsTxBusy(void);

/**
 * @brief Register callback functions
 * @param rxCb Receive callback function
 * @param txCb Transmit complete callback function
 * @param errorCb Error callback function
 */
void UART2_RegisterCallbacks(UART2_RxCallback_t rxCb, 
                           UART2_TxCallback_t txCb,
                           UART2_ErrorCallback_t errorCb);

/**
 * @brief Abort ongoing transmission
 */
void UART2_AbortTransmit(void);

/**
 * @brief Abort ongoing reception
 */
void UART2_AbortReceive(void);

/**
 * @brief Flush all buffers
 * @return UART2_Status_t Status of operation
 */
UART2_Status_t UART2_Flush(void);


#ifdef __cplusplus
}
#endif

#endif /* __USART2_H__ */