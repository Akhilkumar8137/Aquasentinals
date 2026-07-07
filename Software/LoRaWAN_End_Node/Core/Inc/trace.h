/**
  ******************************************************************************
  * @file    trace.h
  * @author  Zero
  * @brief   Public API for the TRACE UART driver.
  *
  * @section how_to_use How to Use:
  *   1. Initialize the desired UART using trace_Init() or trace_InitAdv().
  *   2. Use trace_Printf() or trace_WriteRaw() for transmission.
  *   3. Optionally switch TX mode at runtime using trace_SetTxMode().
  *   4. Start reception by calling trace_StartRx() with a configured
  *      Trace_RxConfig_t structure (choose BYTE, LINE, BUFFER, or CUSTOM mode).
  *   5. Retrieve received data using trace_Available() and trace_RxRead(), or
  *      process data via callbacks.
  *   6. Call trace_Deinit() to deinitialize and release resources.
  ******************************************************************************
  */

#ifndef INC_TRACE_H_
#define INC_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/* Transmission modes */

enum {
    TRACE_TX_MODE_POLLING = 0,
    TRACE_TX_MODE_INTERRUPT,
    TRACE_TX_MODE_DMA
};

/* Reception processing modes */

typedef enum {
    TRACE_RX_PROCESS_BYTE,    // Per-byte callback (interrupt)
    TRACE_RX_PROCESS_LINE,    // Accumulate until terminator, then line callback (interrupt)
    TRACE_RX_PROCESS_BUFFER,  // Chunk-based DMA mode
    TRACE_RX_PROCESS_CUSTOM   // Custom callback
} Trace_RxProcessMode_t;

/* Internal reception hardware modes */

typedef enum {
    TRACE_HW_MODE_POLLING = 0,
    TRACE_HW_MODE_INTERRUPT,
    TRACE_HW_MODE_DMA
} Trace_HwMode_t;

/* Error codes for the TRACE API */

typedef enum {
    TRACE_SUCCESS = 0,
    TRACE_ERROR_UNSUPPORTED_INSTANCE = -1,
    TRACE_ERROR_CONFIG = -2,
    TRACE_ERROR_INIT = -3,
    TRACE_ERROR_DMA_INIT = -4,
    TRACE_ERROR_INVALID_DMA_MODE = -5,
    TRACE_ERROR_INVALID_PARAMETER = -6,
    TRACE_ERROR_MALLOC_FAILED = -7,
    TRACE_ERROR_HARDWARE_FAILURE = -8,
    TRACE_ERROR_BUSY = -9,
    TRACE_ERROR_INVALID_MODE = -10,
    TRACE_ERROR_BUFFER_FULL = -11,
    TRACE_ERROR_NO_DATA = -12,
    TRACE_ERROR_NOT_READY = -13
} Trace_Error_t;

/* DMA configuration modes */

typedef enum {
    TRACE_DMA_MODE_NONE = 0,
    TRACE_DMA_MODE_RX,
    TRACE_DMA_MODE_TX,
    TRACE_DMA_MODE_RX_TX,
} Trace_DMA_Mode_t;

/* Advanced UART initialization structure */

typedef struct {
    USART_TypeDef *Instance;        /**< Pointer to the USART instance. */
    uint32_t BaudRate;              /**< Baud rate for communication. */
    uint32_t WordLength;            /**< Data word length. */
    uint32_t StopBits;              /**< Stop bits configuration. */
    uint32_t Parity;                /**< Parity configuration. */
    uint32_t Mode;                  /**< TX/RX mode. */
    uint32_t HwFlowCtl;             /**< Hardware flow control setting. */
    uint32_t OverSampling;          /**< Oversampling rate. */
    uint32_t OneBitSampling;        /**< One-bit sampling enable/disable. */
    uint32_t AdvFeatureInit;        /**< Advanced feature initialization. */
    uint8_t DMA_Mode;             /**< DMA mode (None, RX, TX, or RX_TX). */
} USART_AdvInit_t;

/* Callback signatures for reception */

typedef void (*Trace_RxByteCallback_t)(uint8_t byte);
typedef void (*Trace_RxLineCallback_t)(const uint8_t* data, uint16_t length);
typedef void (*Trace_RxBufferCallback_t)(uint8_t* buffer, uint16_t len);
typedef void (*Trace_RxCustomCallback_t)(UART_HandleTypeDef *huart, uint8_t* buffer, uint16_t len);

/**
  * @brief  RX configuration structure for the TRACE driver.
  */

typedef struct {
    uint8_t *buffer;               /**< Pointer to the RX ring buffer. */
    uint16_t buffer_size;          /**< Size of the RX ring buffer. */
    uint8_t terminator;            /**< Terminator character (used in LINE mode). */
    Trace_RxProcessMode_t process_mode; /**< Selected RX processing mode. */
    Trace_HwMode_t custom_hw_mode; /**< Selected RX processing mode. */
    union {
        Trace_RxByteCallback_t  byte_cb;   /**< Callback for BYTE mode. */
        Trace_RxLineCallback_t  line_cb;   /**< Callback for LINE mode. */
        Trace_RxBufferCallback_t buffer_cb; /**< Callback for BUFFER mode (DMA). */
        Trace_RxCustomCallback_t custom_cb; /**< Callback for CUSTOM mode. */
    };
} Trace_RxConfig_t;

/* ============================== */
/*  INITIALIZATION FUNCTION PROTOTYPES  */
/* ============================== */

/**
  * @brief  Initializes the specified UART with a default configuration.
  * @param  uart_instance: Pointer to the USART instance.
  * @param  baudRate: Desired baud rate (300 to 2 Mbps).
  * @retval Trace_Error_t: Operation status.
  *
  * This function sets up the UART with default settings (8-bit, 1 stop bit,
  * no parity) and disables DMA. Use trace_InitAdv() for advanced configuration.
  */
Trace_Error_t trace_Init(USART_TypeDef *uart_instance, uint32_t baudRate);
/**
  * @brief  Initializes the UART with advanced configuration.
  * @param  advConfig: Pointer to the advanced initialization structure.
  * @retval Trace_Error_t: Operation status.
  *
  * This function allows full control over UART settings, including DMA mode,
  * word length, parity, and advanced features.
  */
Trace_Error_t trace_InitAdv(USART_AdvInit_t *advConfig);

/* ============================== */
/*  TRANSMISSION FUNCTION PROTOTYPES  */
/* ============================== */

/**
  * @brief  Formatted transmission function.
  * @param  huart: Pointer to the UART handle.
  * @param  format: Format string.
  * @retval Trace_Error_t: Operation status.
  *
  * This function formats data using an internal buffer and transmits it.
  */

Trace_Error_t trace_Printf(UART_HandleTypeDef *huart, const char *format, ...);

/**
  * @brief  Low-level raw write function.
  * @param  huart: Pointer to the UART handle.
  * @param  data: Pointer to the data buffer.
  * @param  length: Number of bytes to transmit.
  * @retval Trace_Error_t: Operation status.
  *
  * This function transmits raw data without formatting.
  */
Trace_Error_t trace_WriteRaw(UART_HandleTypeDef *huart, const uint8_t *data, uint16_t length);

/**
  * @brief  Sets the TX mode for the specified UART.
  * @param  huart: Pointer to the UART handle.
  * @param  mode: Desired TX mode (polling, interrupt, or DMA).
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_SetTxMode(UART_HandleTypeDef *huart, uint8_t mode);

/**
  * @brief  TX complete callback; updates internal TX state.
  * @param  huart: Pointer to the UART handle.
  */
void trace_TxCpltCallback(UART_HandleTypeDef *huart);

/**
  * @brief  Checks if a transmission is ongoing.
  * @param  huart: Pointer to the UART handle.
  * @retval true if transmission is ongoing, false otherwise.
  */
bool is_uart_transmission_ongoing(UART_HandleTypeDef *huart);

/* ============================== */
/*  RECEPTION FUNCTION PROTOTYPES  */
/* ============================== */

/**
  * @brief  Starts reception with the specified configuration.
  * @param  huart: Pointer to the UART handle.
  * @param  config: Pointer to the RX configuration structure.
  * @retval Trace_Error_t: Operation status.
  *
  * This function sets up the RX ring buffer and begins reception using either
  * interrupt (for BYTE, LINE, or CUSTOM modes) or DMA (for BUFFER mode) based on
  * the selected process_mode.
  */

Trace_Error_t trace_StartRx(UART_HandleTypeDef *huart, const Trace_RxConfig_t *config);

/**
  * @brief  Stops any ongoing reception.
  * @param  huart: Pointer to the UART handle.
  * @retval Trace_Error_t: Operation status.
  */

Trace_Error_t trace_StopRx(UART_HandleTypeDef *huart);

/**
  * @brief  Returns the number of bytes available in the RX ring buffer.
  * @param  huart: Pointer to the UART handle.
  * @retval Number of bytes available.
  */

uint16_t trace_Available(UART_HandleTypeDef *huart);

/**
  * @brief  Flushes the RX ring buffer.
  * @param  huart: Pointer to the UART handle.
  */
void trace_Flush(UART_HandleTypeDef *huart);

/**
  * @brief  Switches the RX processing mode at runtime.
  * @param  huart: Pointer to the UART handle.
  * @param  mode: New RX process mode.
  * @param  callback: Pointer to the new callback function.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_SetProcessingMode(UART_HandleTypeDef *huart, Trace_RxProcessMode_t mode, void *callback);

/**
  * @brief  Polls the UART for new data in polling mode.
  * @param  huart: Pointer to the UART handle.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_PollData(UART_HandleTypeDef *huart);

/**
  * @brief  Reads data from the RX ring buffer.
  * @param  huart: Pointer to the UART handle.
  * @param  dest: Destination buffer.
  * @param  length: Maximum number of bytes to read.
  * @retval Number of bytes read.
  */
uint16_t trace_RxRead(UART_HandleTypeDef *huart, uint8_t *dest, uint16_t length);

/**
  * @brief  Unified reception callback.
  * @param  huart: Pointer to the UART handle.
  *
  * This function is called by HAL UART Rx callbacks to process received data
  * based on the current configuration.
  */
void trace_RxCallback(UART_HandleTypeDef *huart);

/* ============================== */
/*  DEINITIALIZATION FUNCTION PROTOTYPES  */
/* ============================== */

/**
  * @brief  Deinitializes the TRACE driver for the specified UART.
  * @param  huart: Pointer to the UART handle.
  * @retval Trace_Error_t: Operation status.
  *
  * This function stops reception, waits for any ongoing transmission to complete,
  * deinitializes the GPIO pins, and shuts down the UART peripheral.
  */
Trace_Error_t trace_Deinit(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* INC_TRACE_H_ */
