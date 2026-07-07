/**
  ******************************************************************************
  * @file    trace_conf.h
  * @author  Zero
  * @brief   Configuration definitions for the TRACE UART driver.
  *
  * This header defines configuration constants, DMA/USART handles,
  * and pin mappings used by the TRACE driver. Include this file in your
  * project to enable low-level definitions for the TRACE API.
  ******************************************************************************
  */


#ifndef INC_TRACE_CONF_H_
#define INC_TRACE_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/** @brief Size of the TX buffer for formatted prints. */
#define TX_BUFFER_SIZE 256

/** @brief Maximum chunk size for one transmission call. */
#define CHUNK_SIZE 128

/** @brief Maximum number of retries for chunked transmissions. */
#define MAX_RETRIES 3


/**
 * @brief Configuration structure describing a particular UART instance.
 */

typedef struct {
    uint32_t tx_pin;              /**< TX GPIO pin. */
    uint32_t rx_pin;              /**< RX GPIO pin. */
    uint32_t alternate_function;  /**< GPIO alternate function setting. */
    IRQn_Type irqn;               /**< USART IRQ number. */
    uint8_t usart_priority;       /**< USART interrupt priority. */
    uint8_t dma_priority;         /**< DMA interrupt priority. */

    DMA_Channel_TypeDef *tx_dma_channel; /**< DMA channel for TX. */
    DMA_Channel_TypeDef *rx_dma_channel; /**< DMA channel for RX. */
    uint32_t dma_request;               /**< DMA request mapping. */
    IRQn_Type dma_irqn;                 /**< DMA IRQ number. */
} Trace_UART_Config;

/**
 * @brief Array of pre-defined UART configurations for supported instances.
 */

const Trace_UART_Config uart_configs[] = {
    [0] = {
        .tx_pin = GPIO_PIN_9,
        .rx_pin = GPIO_PIN_10,
        .alternate_function = GPIO_AF4_USART1,
        .irqn = USART1_IRQn,
        .usart_priority = 0,
        .tx_dma_channel = DMA1_Channel4,
        .rx_dma_channel = DMA1_Channel5,
        .dma_request = DMA_REQUEST_3,
		.dma_priority = 0,
		.dma_irqn = DMA1_Channel4_5_6_7_IRQn
    },
    [1] = {
        .tx_pin = GPIO_PIN_2,
        .rx_pin = GPIO_PIN_3,
        .alternate_function = GPIO_AF4_USART2,
        .irqn = USART2_IRQn,
        .usart_priority = 0,
        .tx_dma_channel = DMA1_Channel7,
        .rx_dma_channel = DMA1_Channel6,
        .dma_request = DMA_REQUEST_4,
		.dma_priority = 0,
		.dma_irqn = DMA1_Channel4_5_6_7_IRQn
    }
};

#ifdef __cplusplus
}
#endif

#endif /* INC_TRACE_CONF_H_ */
