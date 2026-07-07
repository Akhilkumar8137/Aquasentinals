/**
  ******************************************************************************
  * @file    trace.c
  * @author  Zero
  * @brief   Implementation of the TRACE UART driver.
  *
  * This file implements the API for a flexible and robust UART driver for STM32,
  * supporting transmission in polling, interrupt, or DMA modes, as well as
  * reception with multiple processing modes (byte, line, buffer, or custom).
  ******************************************************************************
  */

#include "trace.h"
#include "stm32_tiny_vsnprintf.h"
#include "trace_conf.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* --- Global Variables --- */

/* Store original DMA mode for each USART instance */
static uint8_t original_dma_mode_usart1 = TRACE_DMA_MODE_NONE;
static uint8_t original_dma_mode_usart2 = TRACE_DMA_MODE_NONE;

/* Transmission state structure */
typedef struct {
    volatile bool tx_busy;
    const uint8_t *data;        // Pointer to full data buffer
    uint16_t total_len;         // Total length to transmit
    uint16_t offset;            // Current offset in data
    uint8_t retries_remaining;  // Retry counter for current chunk
} TxState_t;

/* Reception state structure using ring buffer mechanism */
typedef struct {
    volatile bool rx_busy;               // Reception active flag
    volatile uint16_t rx_head;           // Next write position (interrupt/polling)
    volatile uint16_t rx_tail;           // Next read position
    uint8_t *buffer;                     // Ring buffer for RX
    uint16_t buffer_size;                // Buffer capacity
    uint8_t terminator;                  // End-of-frame character (for buffer mode)
    Trace_RxProcessMode_t process_mode;  // Processing mode: BYTE, BUFFER, CUSTOM
    Trace_HwMode_t hw_mode;              // Actual hardware mode used for RX
    union {
        Trace_RxByteCallback_t   byte_cb;
        Trace_RxLineCallback_t   line_cb;    ///< Callback for line mode
        Trace_RxBufferCallback_t buffer_cb;
        Trace_RxCustomCallback_t custom_cb;
    };
    /* For DMA mode */
    volatile uint16_t last_dma_index;    // Last processed index in DMA buffer
} RxState_t;

/* Static state instances for USART1 and USART2 */
static TxState_t tx_state_usart1 = { .tx_busy = false };
static TxState_t tx_state_usart2 = { .tx_busy = false };

static RxState_t rx_states[2] = {0};

/* Current TX mode for each UART (default is interrupt) */
static uint8_t current_tx_mode_usart1 = TRACE_TX_MODE_INTERRUPT;
static uint8_t current_tx_mode_usart2 = TRACE_TX_MODE_INTERRUPT;

/* Transmission print buffer */
static char tx_print_buffer[TX_BUFFER_SIZE];

/* ============================== */
/*        HELPER FUNCTIONS        */
/* ============================== */
/**
  * @brief  Returns a pointer to the RX state for the given UART.
  * @param  huart: Pointer to the UART handle.
  * @retval Pointer to the corresponding RxState_t.
  */
static RxState_t* get_rx_state(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) return (&rx_states[0]);
    if (huart->Instance == USART2) return (&rx_states[1]);
    return NULL;
}

/**
  * @brief  Returns a pointer to the TX state for the given UART.
  * @param  huart: Pointer to the UART handle.
  * @retval Pointer to the corresponding TxState_t.
  */
static TxState_t *get_tx_state(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) return (&tx_state_usart1);
    if (huart->Instance == USART2) return (&tx_state_usart2);
    return NULL;
}


/**
  * @brief  Returns the current TX mode for the given UART.
  * @param  huart: Pointer to the UART handle.
  * @retval TX mode (polling, interrupt, or DMA).
  */
static uint8_t get_current_tx_mode(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) return (current_tx_mode_usart1);
    if (huart->Instance == USART2) return (current_tx_mode_usart2);
    return (TRACE_TX_MODE_POLLING);
}

/**
  * @brief  Checks if TX is enabled in the UART configuration.
  * @param  huart: Pointer to the UART handle.
  * @retval true if TX is enabled, false otherwise.
  */
static bool is_uart_tx_enabled(UART_HandleTypeDef *huart) {
    return ((huart->Init.Mode & UART_MODE_TX) != 0 ||
            (huart->Init.Mode & UART_MODE_TX_RX) != 0);
}

/**
  * @brief  Checks if RX is enabled in the UART configuration.
  * @param  huart: Pointer to the UART handle.
  * @retval true if RX is enabled, false otherwise.
  */
static bool is_uart_rx_enabled(UART_HandleTypeDef *huart) {
    return ((huart->Init.Mode & UART_MODE_RX) != 0 ||
            (huart->Init.Mode & UART_MODE_TX_RX) != 0);
}

/**
  * @brief  Ensures the UART TX mode is enabled and reinitializes if necessary.
  * @param  huart: Pointer to the UART handle.
  * @retval Trace_Error_t: Operation status.
  */
static Trace_Error_t ensure_uart_tx_mode(UART_HandleTypeDef *huart) {
    if (is_uart_tx_enabled(huart)) {
        uint8_t orig_dma_mode = TRACE_DMA_MODE_NONE;

        if (huart->Instance == USART1) {
            orig_dma_mode = original_dma_mode_usart1;
        } else if (huart->Instance == USART2) {
            orig_dma_mode = original_dma_mode_usart2;
        }

        if (orig_dma_mode == TRACE_DMA_MODE_NONE) {
            return (TRACE_SUCCESS);
        }
    }
    huart->Init.Mode |= UART_MODE_TX_RX;

    USART_AdvInit_t advConfig = {0};
    advConfig.Instance = huart->Instance;
    advConfig.BaudRate = huart->Init.BaudRate;
    advConfig.WordLength = huart->Init.WordLength;
    advConfig.StopBits = huart->Init.StopBits;
    advConfig.Parity = huart->Init.Parity;
    advConfig.Mode = huart->Init.Mode;
    advConfig.HwFlowCtl = huart->Init.HwFlowCtl;
    advConfig.OverSampling = huart->Init.OverSampling;
    advConfig.OneBitSampling = huart->Init.OneBitSampling;
    advConfig.AdvFeatureInit = huart->AdvancedInit.AdvFeatureInit;

    uint8_t orig_dma_mode = TRACE_DMA_MODE_NONE;
    if (huart->Instance == USART1) {
        orig_dma_mode = original_dma_mode_usart1;
    } else if (huart->Instance == USART2) {
        orig_dma_mode = original_dma_mode_usart2;
    }
	if (get_current_tx_mode(huart) == TRACE_TX_MODE_DMA) {
		if ((orig_dma_mode != TRACE_DMA_MODE_TX)
				&& (orig_dma_mode != TRACE_DMA_MODE_RX_TX)) {
			advConfig.DMA_Mode = TRACE_DMA_MODE_RX_TX;
		} else{
			advConfig.DMA_Mode = orig_dma_mode;
		}
	}

    return (trace_InitAdv(&advConfig));
}

/**
  * @brief  Ensures the UART RX mode is enabled and reinitializes if necessary.
  * @param  huart: Pointer to the UART handle.
  * @retval Trace_Error_t: Operation status.
  */
static Trace_Error_t ensure_uart_rx_mode(UART_HandleTypeDef *huart) {
    if (is_uart_rx_enabled(huart)) {
        uint8_t orig_dma_mode = TRACE_DMA_MODE_NONE;
        if (huart->Instance == USART1) {
            orig_dma_mode = original_dma_mode_usart1;
        } else if (huart->Instance == USART2) {
            orig_dma_mode = original_dma_mode_usart2;
        }
        if (orig_dma_mode == TRACE_DMA_MODE_NONE) {
            return (TRACE_SUCCESS);
        }
    }
    huart->Init.Mode |= UART_MODE_TX_RX;
    USART_AdvInit_t advConfig = {0};
    advConfig.Instance = huart->Instance;
    advConfig.BaudRate = huart->Init.BaudRate;
    advConfig.WordLength = huart->Init.WordLength;
    advConfig.StopBits = huart->Init.StopBits;
    advConfig.Parity = huart->Init.Parity;
    advConfig.Mode = huart->Init.Mode;
    advConfig.HwFlowCtl = huart->Init.HwFlowCtl;
    advConfig.OverSampling = huart->Init.OverSampling;
    advConfig.OneBitSampling = huart->Init.OneBitSampling;
    advConfig.AdvFeatureInit = huart->AdvancedInit.AdvFeatureInit;

    RxState_t *state = get_rx_state(huart);
    if (!state) {
        return (TRACE_ERROR_UNSUPPORTED_INSTANCE);
    }

    uint8_t orig_dma_mode = TRACE_DMA_MODE_NONE;
    if (huart->Instance == USART1) {
        orig_dma_mode = original_dma_mode_usart1;
    } else if (huart->Instance == USART2) {
        orig_dma_mode = original_dma_mode_usart2;
    }

    if (state->hw_mode == TRACE_HW_MODE_DMA) {
        if ((orig_dma_mode != TRACE_DMA_MODE_RX) && (orig_dma_mode != TRACE_DMA_MODE_RX_TX)) {
            advConfig.DMA_Mode = TRACE_DMA_MODE_RX_TX;
        } else {
            advConfig.DMA_Mode = orig_dma_mode;
        }
    }
    return (trace_InitAdv(&advConfig));
}

/**
  * @brief  Enables DMA for UART TX.
  * @param  huart: Pointer to the UART handle.
  * @param  config: Pointer to the UART configuration.
  * @retval Trace_Error_t: Operation status.
  */

static Trace_Error_t enable_DMA_TX(UART_HandleTypeDef *huart, const Trace_UART_Config *config) {
    DMA_HandleTypeDef *hdma_tx = NULL;
    __DMA1_CLK_ENABLE();
    if (huart->Instance == USART1) {
        hdma_tx = &hdma_usart1_tx;
    } else if (huart->Instance == USART2) {
        hdma_tx = &hdma_usart2_tx;
    }
    if (hdma_tx == NULL) {
        return (TRACE_ERROR_UNSUPPORTED_INSTANCE);
    }
    hdma_tx->Instance = config->tx_dma_channel;
    hdma_tx->Init.Request = config->dma_request;
    hdma_tx->Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tx->Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tx->Init.MemInc = DMA_MINC_ENABLE;
    hdma_tx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_tx->Init.Mode = DMA_NORMAL; // TX does not require circular mode
    hdma_tx->Init.Priority = config->dma_priority;
    if (HAL_DMA_Init(hdma_tx) != HAL_OK) {
        return (TRACE_ERROR_DMA_INIT);
    }
    HAL_NVIC_SetPriority(config->dma_irqn, config->dma_priority, 0);
    HAL_NVIC_EnableIRQ(config->dma_irqn);
    __HAL_LINKDMA(huart, hdmatx, *hdma_tx);
    return (TRACE_SUCCESS);

}

/**
  * @brief  Enables DMA for UART RX.
  * @param  huart: Pointer to the UART handle.
  * @param  config: Pointer to the UART configuration.
  * @retval Trace_Error_t: Operation status.
  */
static Trace_Error_t enable_DMA_RX(UART_HandleTypeDef *huart, const Trace_UART_Config *config) {
    DMA_HandleTypeDef *hdma_rx = NULL;
    __DMA1_CLK_ENABLE();
    if (huart->Instance == USART1) {
        hdma_rx = &hdma_usart1_rx;
    } else if (huart->Instance == USART2) {
        hdma_rx = &hdma_usart2_rx;
    }
    if (hdma_rx == NULL) {
        return (TRACE_ERROR_UNSUPPORTED_INSTANCE);
    }
    hdma_rx->Instance = config->rx_dma_channel;
    hdma_rx->Init.Request = config->dma_request;
    hdma_rx->Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_rx->Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_rx->Init.MemInc = DMA_MINC_ENABLE;
    hdma_rx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_rx->Init.Mode = DMA_CIRCULAR; // Use circular mode for continuous ring buffer RX
    hdma_rx->Init.Priority = config->dma_priority;
    if (HAL_DMA_Init(hdma_rx) != HAL_OK) {
        return (TRACE_ERROR_DMA_INIT);
    }
    HAL_NVIC_SetPriority(config->dma_irqn, config->dma_priority, 0);
    HAL_NVIC_EnableIRQ(config->dma_irqn);
    __HAL_LINKDMA(huart, hdmarx, *hdma_rx);
    return (TRACE_SUCCESS);
}

/* ============================== */
/*     TRANSMISSION FUNCTION      */
/* ============================== */

/**
  * @brief  Sends a data chunk based on the current TX mode.
  * @param  huart: Pointer to the UART handle.
  * @retval HAL_StatusTypeDef: HAL status.
  */
static HAL_StatusTypeDef Send_Chunk(UART_HandleTypeDef *huart) {
    HAL_StatusTypeDef status = HAL_OK;
    TxState_t *state = get_tx_state(huart);
    uint16_t remaining = state->total_len - state->offset;
    uint16_t chunk_len = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
    switch (get_current_tx_mode(huart)) {
        case TRACE_TX_MODE_DMA:
            status = HAL_UART_Transmit_DMA(huart, (uint8_t *)&state->data[state->offset], chunk_len);
            break;
        case TRACE_TX_MODE_INTERRUPT:
            status = HAL_UART_Transmit_IT(huart, (uint8_t *)&state->data[state->offset], chunk_len);
            break;
        case TRACE_TX_MODE_POLLING:
            status = HAL_UART_Transmit(huart, (uint8_t *)&state->data[state->offset], chunk_len, HAL_MAX_DELAY);
            state->offset += chunk_len;
            break;
        default:
            break;
    }
    return (status);
}

/**
  * @brief  Transmits data with chunking and retry logic.
  * @param  huart: Pointer to the UART handle.
  * @param  data: Pointer to the data to transmit.
  * @param  length: Length of data to transmit.
  * @retval Trace_Error_t: Operation status.
  */
static Trace_Error_t Trace_Send(UART_HandleTypeDef *huart, const uint8_t *data, uint16_t length) {
    TxState_t *state = get_tx_state(huart);
    if (state->tx_busy) {
        return (TRACE_ERROR_BUSY);
    }
    state->data = data;
    state->total_len = length;
    state->offset = 0;
    state->retries_remaining = MAX_RETRIES;
    state->tx_busy = true;
    if (Send_Chunk(huart) != HAL_OK) {
        state->tx_busy = false;
        return (TRACE_ERROR_HARDWARE_FAILURE);
    }
    if (get_current_tx_mode(huart) == TRACE_TX_MODE_POLLING) {
        while (state->offset < state->total_len) {
            if (Send_Chunk(huart) != HAL_OK) {
                state->tx_busy = false;
                return (TRACE_ERROR_HARDWARE_FAILURE);
            }
        }
        state->tx_busy = false;
    }
    return (TRACE_SUCCESS);
}

/**
  * @brief  Initializes UART with default advanced configuration.
  * @param  uart_instance: Pointer to the USART instance.
  * @param  baudRate: Desired baud rate.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_Init(USART_TypeDef *uart_instance, uint32_t baudRate) {
  if (baudRate < 300 || baudRate > 2000000) {
    return (TRACE_ERROR_INVALID_PARAMETER);
  }

  USART_AdvInit_t advConfig = {0};
  advConfig.Instance = uart_instance;
  advConfig.BaudRate = baudRate;
  advConfig.WordLength = UART_WORDLENGTH_8B;
  advConfig.StopBits = UART_STOPBITS_1;
  advConfig.Parity = UART_PARITY_NONE;
  advConfig.Mode = UART_MODE_TX_RX;
  advConfig.HwFlowCtl = UART_HWCONTROL_NONE;
  advConfig.OverSampling = UART_OVERSAMPLING_16;
  advConfig.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  advConfig.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  advConfig.DMA_Mode = TRACE_DMA_MODE_NONE;

  Trace_Error_t status = trace_InitAdv(&advConfig);
  if (status != TRACE_SUCCESS) {
    return (status);
  }

  return (TRACE_SUCCESS);
}

/**
  * @brief  Advanced UART initialization.
  * @param  advConfig: Pointer to the advanced initialization structure.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_InitAdv(USART_AdvInit_t *advConfig) {
  UART_HandleTypeDef *huart = NULL;
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  const Trace_UART_Config *config = NULL;

  if (advConfig == NULL || advConfig->Instance == NULL) {
    return (TRACE_ERROR_INVALID_PARAMETER);
  }

  if (advConfig->Instance == USART1) {
    config = &uart_configs[0];
    __HAL_RCC_USART1_CLK_ENABLE();
    huart = &huart1;
  } else if (advConfig->Instance == USART2) {
    config = &uart_configs[1];
    __HAL_RCC_USART2_CLK_ENABLE();
    huart = &huart2;
  } else {
    return (TRACE_ERROR_UNSUPPORTED_INSTANCE);
  }

  if (config == NULL) {
    return (TRACE_ERROR_CONFIG);
  }

  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin = config->tx_pin | config->rx_pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = config->alternate_function;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  huart->Instance = advConfig->Instance;
  huart->Init.BaudRate = advConfig->BaudRate;
  huart->Init.WordLength = advConfig->WordLength;
  huart->Init.StopBits = advConfig->StopBits;
  huart->Init.Parity = advConfig->Parity;
  huart->Init.Mode = advConfig->Mode;
  huart->Init.HwFlowCtl = advConfig->HwFlowCtl;
  huart->Init.OverSampling = advConfig->OverSampling;
  huart->Init.OneBitSampling = advConfig->OneBitSampling;
  huart->AdvancedInit.AdvFeatureInit = advConfig->AdvFeatureInit;

  if (HAL_UART_Init(huart) != HAL_OK) {
    return (TRACE_ERROR_HARDWARE_FAILURE);

  }

  HAL_NVIC_SetPriority(config->irqn, config->usart_priority, 0);
  HAL_NVIC_EnableIRQ(config->irqn);


  switch (advConfig->DMA_Mode) {
    case TRACE_DMA_MODE_NONE:
      break;
    case TRACE_DMA_MODE_TX:
      if (enable_DMA_TX(huart, config) != TRACE_SUCCESS) {
        return (TRACE_ERROR_DMA_INIT);
      }
      break;
    case TRACE_DMA_MODE_RX:
      if (enable_DMA_RX(huart, config) != TRACE_SUCCESS) {
        return (TRACE_ERROR_DMA_INIT);
      }
      break;
    case TRACE_DMA_MODE_RX_TX:
      if (enable_DMA_TX(huart, config) != TRACE_SUCCESS ||
          enable_DMA_RX(huart, config) != TRACE_SUCCESS) {
        return (TRACE_ERROR_DMA_INIT);
      }
      break;
    default:
      return (TRACE_ERROR_INVALID_DMA_MODE);
  }

  if (advConfig->Instance == USART1) {
    original_dma_mode_usart1 = advConfig->DMA_Mode;
  } else if (advConfig->Instance == USART2) {
    original_dma_mode_usart2 = advConfig->DMA_Mode;
  }

  return (TRACE_SUCCESS);
}

/**
  * @brief  Formatted transmission function.
  * @param  huart: Pointer to the UART handle.
  * @param  format: Format string.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_Printf(UART_HandleTypeDef *huart, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int len = tiny_vsnprintf_like(tx_print_buffer, sizeof(tx_print_buffer), format, args);
    va_end(args);
    if (len < 0) {
        return (TRACE_ERROR_INVALID_PARAMETER);
    }
    if (len > TX_BUFFER_SIZE) {
        len = TX_BUFFER_SIZE;
        tx_print_buffer[TX_BUFFER_SIZE - 1] = '\0';
    }
    return (trace_WriteRaw(huart, (const uint8_t *)tx_print_buffer, (uint16_t)len));
}

/**
  * @brief  Low-level raw write function.
  * @param  huart: Pointer to the UART handle.
  * @param  data: Data buffer.
  * @param  length: Number of bytes to send.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_WriteRaw(UART_HandleTypeDef *huart, const uint8_t *data, uint16_t length) {
    if (huart == NULL || data == NULL || length == 0) {
        return (TRACE_ERROR_INVALID_PARAMETER);
    }
    if (!is_uart_tx_enabled(huart)) {
        Trace_Error_t status = ensure_uart_tx_mode(huart);
        if (status != TRACE_SUCCESS) {
            return (status);
        }
    }
    return (Trace_Send(huart, data, length));
}

/**
  * @brief  Sets the transmission mode.
  * @param  huart: Pointer to the UART handle.
  * @param  mode: Desired TX mode.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_SetTxMode(UART_HandleTypeDef *huart, uint8_t mode) {
    if (huart == NULL) {
        return (TRACE_ERROR_INVALID_PARAMETER);
    }
    if (mode != TRACE_TX_MODE_INTERRUPT && mode != TRACE_TX_MODE_DMA && mode != TRACE_TX_MODE_POLLING) {
        return (TRACE_ERROR_INVALID_PARAMETER);
    }
    if (huart->Instance == USART1) {
        current_tx_mode_usart1 = mode;
    } else if (huart->Instance == USART2) {
        current_tx_mode_usart2 = mode;
    } else {
        return (TRACE_ERROR_UNSUPPORTED_INSTANCE);
    }
    if (mode == TRACE_TX_MODE_DMA) {
		if (huart->Instance == USART1) {
			if (original_dma_mode_usart1 != TRACE_DMA_MODE_TX
					&& original_dma_mode_usart1 != TRACE_DMA_MODE_RX_TX) {
				original_dma_mode_usart1 = TRACE_DMA_MODE_RX_TX;
				ensure_uart_tx_mode(huart);
			}
		}
    	else if (huart->Instance == USART2) {
			if (original_dma_mode_usart2 != TRACE_DMA_MODE_TX
					&& original_dma_mode_usart2 != TRACE_DMA_MODE_RX_TX) {
				original_dma_mode_usart2 = TRACE_DMA_MODE_RX_TX;
				ensure_uart_tx_mode(huart);
			}
		}
    }
    return (TRACE_SUCCESS);
}


/**
  * @brief  TX complete callback; updates the TX state.
  * @param  huart: Pointer to the UART handle.
  */
void trace_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (!huart) return;
    TxState_t *state = get_tx_state(huart);
    if (!state || !state->tx_busy) return;
    uint16_t remaining = state->total_len - state->offset;
    uint16_t chunk_len = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
    state->offset += chunk_len;
    if (state->offset < state->total_len) {
        state->retries_remaining = MAX_RETRIES;
        if (Send_Chunk(huart) != HAL_OK) {
            if (state->retries_remaining-- > 0) {
                Send_Chunk(huart);  // Retry
            } else {
                state->tx_busy = false;
            }
        }
    } else {
        state->tx_busy = false;
    }
}

/**
  * @brief  Checks if a transmission is ongoing.
  * @param  huart: Pointer to the UART handle.
  * @retval true if transmission is ongoing, false otherwise.
  */
bool is_uart_transmission_ongoing(UART_HandleTypeDef *huart) {
    TxState_t *state = get_tx_state(huart);
    return ((state != NULL) ? state->tx_busy : false);
}

/* ============================== */
/*  RECEPTION FUNCTION            */
/* ============================== */

/**
  * @brief  Starts reception based on the provided configuration.
  * @param  huart: Pointer to the UART handle.
  * @param  config: Pointer to the RX configuration structure.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_StartRx(UART_HandleTypeDef *huart, const Trace_RxConfig_t *config)
{
    if (!huart || !config || !config->buffer || config->buffer_size == 0)
        return (TRACE_ERROR_INVALID_PARAMETER);

    RxState_t *state = get_rx_state(huart);
    if (!state)
        return (TRACE_ERROR_UNSUPPORTED_INSTANCE);

    trace_StopRx(huart);

    state->buffer      = config->buffer;
    state->buffer_size = config->buffer_size;
    state->terminator  = config->terminator;
    state->process_mode= config->process_mode;
    state->rx_head     = 0;
    state->rx_tail     = 0;
    state->rx_busy     = true;
    state->last_dma_index = 0;

    switch (config->process_mode) {
    case TRACE_RX_PROCESS_BYTE:
        state->hw_mode = TRACE_HW_MODE_INTERRUPT;
        state->byte_cb = config->byte_cb;
        break;

    case TRACE_RX_PROCESS_LINE:
        state->hw_mode = TRACE_HW_MODE_INTERRUPT;
        state->line_cb = config->line_cb;
        break;

    case TRACE_RX_PROCESS_BUFFER:
        state->hw_mode = TRACE_HW_MODE_DMA;
        state->buffer_cb = config->buffer_cb;
        break;

    case TRACE_RX_PROCESS_CUSTOM:
    	state->hw_mode = config->custom_hw_mode;
        state->custom_cb = config->custom_cb;
        break;

    default:
        return (TRACE_ERROR_INVALID_MODE);
    }

	if (state->hw_mode == TRACE_HW_MODE_DMA) {
		if (huart->Instance == USART1) {
			if (original_dma_mode_usart1 == TRACE_DMA_MODE_NONE)
				original_dma_mode_usart1 = TRACE_DMA_MODE_RX;
			else {
				original_dma_mode_usart1 = TRACE_DMA_MODE_RX_TX;

			}
		} else if (huart->Instance == USART2) {
			if (original_dma_mode_usart2 == TRACE_DMA_MODE_NONE)
				original_dma_mode_usart2 = TRACE_DMA_MODE_RX;
			else {
				original_dma_mode_usart2 = TRACE_DMA_MODE_RX_TX;

			}
		}
	}

    Trace_Error_t status = ensure_uart_rx_mode(huart);
    if (status != TRACE_SUCCESS) {
        state->rx_busy = false;
        return (status);
    }

    if (state->hw_mode == TRACE_HW_MODE_INTERRUPT) {
        if (HAL_UART_Receive_IT(huart, &state->buffer[state->rx_head], 1) != HAL_OK) {
            state->rx_busy = false;
            return (TRACE_ERROR_HARDWARE_FAILURE);
        }
    }
    else if (state->hw_mode == TRACE_HW_MODE_DMA) {
        if (HAL_UART_Receive_DMA(huart, state->buffer, state->buffer_size) != HAL_OK) {
            state->rx_busy = false;
            return (TRACE_ERROR_HARDWARE_FAILURE);
        }
    }

    return (TRACE_SUCCESS);
}

/**
  * @brief  Stops the ongoing reception.
  * @param  huart: Pointer to the UART handle.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_StopRx(UART_HandleTypeDef *huart)
{
    if (!huart)
        return (TRACE_ERROR_INVALID_PARAMETER);
    RxState_t *state = get_rx_state(huart);
    if (!state)
        return (TRACE_ERROR_UNSUPPORTED_INSTANCE);
    state->rx_busy = false;

    if (state->hw_mode == TRACE_HW_MODE_DMA)
        HAL_UART_DMAStop(huart);
    if (state->hw_mode == TRACE_HW_MODE_INTERRUPT)
        HAL_UART_AbortReceive_IT(huart);
    return (TRACE_SUCCESS);
}

/**
  * @brief  Returns the number of bytes available in the RX ring buffer.
  * @param  huart: Pointer to the UART handle.
  * @retval Number of available bytes.
  */
uint16_t trace_Available(UART_HandleTypeDef *huart)
{
    RxState_t *state = get_rx_state(huart);
    if (!state)
        return (0);

    if (state->hw_mode == TRACE_HW_MODE_DMA) {
        uint16_t dma_head = state->buffer_size - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        dma_head %= state->buffer_size;
        if (dma_head >= state->rx_tail)
            return (dma_head - state->rx_tail);
        else
            return (state->buffer_size - state->rx_tail + dma_head);
    } else {
        if (state->rx_head >= state->rx_tail)
            return (state->rx_head - state->rx_tail);
        else
            return (state->buffer_size - state->rx_tail + state->rx_head);
    }
}

/**
  * @brief  Flushes the RX ring buffer.
  * @param  huart: Pointer to the UART handle.
  */
void trace_Flush(UART_HandleTypeDef *huart)
{
    RxState_t *state = get_rx_state(huart);
    if (!state)
        return;
    state->rx_head = 0;
    state->rx_tail = 0;
    if (state->buffer && state->buffer_size)
        memset(state->buffer, 0, state->buffer_size);
    if (state->hw_mode == TRACE_HW_MODE_DMA)
        state->last_dma_index = 0;
}

/**
  * @brief  Reads data from the RX ring buffer.
  * @param  huart: Pointer to the UART handle.
  * @param  dest: Destination buffer.
  * @param  length: Maximum number of bytes to read.
  * @retval Number of bytes read.
  */
uint16_t trace_RxRead(UART_HandleTypeDef *huart, uint8_t *dest, uint16_t length)
{
    if (!huart || !dest)
        return (0);
    RxState_t *state = get_rx_state(huart);
    if (!state)
        return (0);
    uint16_t available = trace_Available(huart);
    uint16_t to_read = (length < available) ? length : available;
    for (uint16_t i = 0; i < to_read; i++) {
        dest[i] = state->buffer[state->rx_tail];
        state->rx_tail = (state->rx_tail + 1) % state->buffer_size;
    }
    return (to_read);
}

/**
  * @brief  For polling mode: collects one byte from the UART.
  * @param  huart: Pointer to the UART handle.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_PollData(UART_HandleTypeDef *huart)
{
    if (!huart)
        return (TRACE_ERROR_INVALID_PARAMETER);
    RxState_t *state = get_rx_state(huart);
    if (!state)
        return (TRACE_ERROR_UNSUPPORTED_INSTANCE);
    if (!state->rx_busy || state->hw_mode != TRACE_HW_MODE_POLLING)
        return (TRACE_ERROR_NOT_READY);

    uint8_t byte;
    HAL_StatusTypeDef hal_status = HAL_UART_Receive(huart, &byte, 1, 0);
    if (hal_status == HAL_OK)
    {
        uint16_t next_head = (state->rx_head + 1) % state->buffer_size;
        if (next_head == state->rx_tail) {
            trace_Flush(huart);
            state->rx_head = 0;
            state->rx_tail = 0;
            next_head = 1;
        }
        state->buffer[state->rx_head] = byte;
        state->rx_head = next_head;

        if (state->process_mode == TRACE_RX_PROCESS_BYTE && state->byte_cb)
            state->byte_cb(byte);
        else if (state->process_mode == TRACE_RX_PROCESS_BUFFER && state->buffer_cb) {
            if (byte == state->terminator) {
                uint16_t len = trace_Available(huart);
                state->buffer_cb(state->buffer, len);
                trace_Flush(huart);
            }
        }
        else if (state->process_mode == TRACE_RX_PROCESS_CUSTOM && state->custom_cb) {
            uint16_t len = trace_Available(huart);
            state->custom_cb(huart, state->buffer, len);
        }
        return (TRACE_SUCCESS);
    }
    return ((hal_status == HAL_TIMEOUT) ? TRACE_ERROR_NO_DATA : TRACE_ERROR_HARDWARE_FAILURE);
}
/**
  * @brief  Unified reception callback, to be called from HAL UART Rx callbacks.
  * @param  huart: Pointer to the UART handle.
  */
void trace_RxCallback(UART_HandleTypeDef *huart)
{
    RxState_t *state = get_rx_state(huart);
    if (!state || !state->rx_busy)
        return;
    if (state->hw_mode == TRACE_HW_MODE_INTERRUPT)
    {
        uint8_t received = state->buffer[state->rx_head];
        state->rx_head = (state->rx_head + 1) % state->buffer_size;
        if (state->rx_head == state->rx_tail)
            trace_Flush(huart);
        if (state->process_mode == TRACE_RX_PROCESS_BYTE && state->byte_cb)
        {
            state->byte_cb(received);
        }
        else if (state->process_mode == TRACE_RX_PROCESS_LINE && state->line_cb)
        {
            if (received == state->terminator && state->terminator != 0)
            {
                uint16_t available = trace_Available(huart);
                uint8_t localBuf[128];
                if (available > sizeof(localBuf))
                    available = sizeof(localBuf);
                uint16_t readCount = trace_RxRead(huart, localBuf, available);
                state->line_cb(localBuf, readCount);
            }
        }
        else if (state->process_mode == TRACE_RX_PROCESS_CUSTOM && state->custom_cb)
        {
            uint16_t len = trace_Available(huart);
            state->custom_cb(huart, state->buffer, len);
        }
        if (HAL_UART_Receive_IT(huart, &state->buffer[state->rx_head], 1) != HAL_OK)
        {
            trace_StopRx(huart);
            Trace_RxConfig_t new_config;
            new_config.buffer = state->buffer;
            new_config.buffer_size = state->buffer_size;
            new_config.terminator = state->terminator;
            new_config.process_mode = state->process_mode;
            if (state->process_mode == TRACE_RX_PROCESS_BYTE)
                new_config.byte_cb = state->byte_cb;
            else if (state->process_mode == TRACE_RX_PROCESS_LINE)
                new_config.line_cb = state->line_cb;
            else if (state->process_mode == TRACE_RX_PROCESS_CUSTOM)
                new_config.custom_cb = state->custom_cb;
            trace_StartRx(huart, &new_config);
        }
    }
    else if (state->hw_mode == TRACE_HW_MODE_DMA)
    {
        uint16_t dma_head = state->buffer_size - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        dma_head %= state->buffer_size;
        uint16_t old_head = state->rx_head;
        while (state->last_dma_index != dma_head)
        {
            state->last_dma_index = (state->last_dma_index + 1) % state->buffer_size;
        }
        state->rx_head = dma_head;

        if (state->process_mode == TRACE_RX_PROCESS_BUFFER && state->buffer_cb)
        {
            uint16_t newBytes = 0;
            if (state->rx_head >= old_head)
                newBytes = state->rx_head - old_head;
            else
                newBytes = (state->buffer_size - old_head) + state->rx_head;
            if (newBytes > 0)
                state->buffer_cb(&state->buffer[old_head], newBytes);
        }
        else if (state->process_mode == TRACE_RX_PROCESS_CUSTOM && state->custom_cb)
        {
            uint16_t totalAvail = trace_Available(huart);
            state->custom_cb(huart, state->buffer, totalAvail);
        }
    }
}


/**
  * @brief  Switches the RX processing mode at runtime.
  * @param  huart: Pointer to the UART handle.
  * @param  mode: New RX process mode.
  * @param  callback: Pointer to the new callback function.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_SetProcessingMode(UART_HandleTypeDef *huart, Trace_RxProcessMode_t mode, void *callback)
{
    if (!huart || !callback)
        return (TRACE_ERROR_INVALID_PARAMETER);
    RxState_t *state = get_rx_state(huart);
    if (!state)
        return (TRACE_ERROR_UNSUPPORTED_INSTANCE);
    Trace_Error_t status = trace_StopRx(huart);
    if (status != TRACE_SUCCESS)
        return (status);
    state->process_mode = mode;
    if (mode == TRACE_RX_PROCESS_BYTE) {
        state->byte_cb = (Trace_RxByteCallback_t)callback;
        state->hw_mode = TRACE_HW_MODE_INTERRUPT;
    }
    else if (mode == TRACE_RX_PROCESS_BUFFER) {
        state->buffer_cb = (Trace_RxBufferCallback_t)callback;
        state->hw_mode = TRACE_HW_MODE_DMA;
    }
    else if (mode == TRACE_RX_PROCESS_CUSTOM) {
        state->custom_cb = (Trace_RxCustomCallback_t)callback;
        state->hw_mode = TRACE_HW_MODE_INTERRUPT;
    }
    else {
        return (TRACE_ERROR_INVALID_MODE);
    }
    Trace_RxConfig_t new_config;
    new_config.buffer = state->buffer;
    new_config.buffer_size = state->buffer_size;
    new_config.terminator = state->terminator;
    new_config.process_mode = state->process_mode;
    new_config.byte_cb = state->byte_cb;
    new_config.buffer_cb = state->buffer_cb;
    new_config.custom_cb = state->custom_cb;
    status = trace_StartRx(huart, &new_config);
    return (status);
}

/**
  * @brief  Deinitializes the TRACE driver for a given UART.
  * @param  huart: Pointer to the UART handle.
  * @retval Trace_Error_t: Operation status.
  */
Trace_Error_t trace_Deinit(UART_HandleTypeDef *huart) {
    const Trace_UART_Config *config = NULL;
    uint32_t timeout;
    if (huart == NULL) {
        return (TRACE_ERROR_INVALID_PARAMETER);
    }
    Trace_Error_t status = trace_StopRx(huart);
    if (status != TRACE_SUCCESS) {
        return (status);
    }
    timeout = HAL_GetTick() + 1000;
    while (is_uart_transmission_ongoing(huart)) {
        if (HAL_GetTick() > timeout) {
            HAL_UART_AbortTransmit(huart);
            break;
        }
    }


    if (huart->Instance == USART1) {
        config = &uart_configs[0];
        __HAL_RCC_USART1_CLK_DISABLE();
    } else if (huart->Instance == USART2) {
        config = &uart_configs[1];
        __HAL_RCC_USART2_CLK_DISABLE();
    } else {
        return (TRACE_ERROR_UNSUPPORTED_INSTANCE);
    }
    HAL_NVIC_DisableIRQ(config->irqn);
    HAL_GPIO_DeInit(GPIOA, config->tx_pin | config->rx_pin);
    if (HAL_UART_DeInit(huart) != HAL_OK) {
        return (TRACE_ERROR_HARDWARE_FAILURE);
    }
    return (TRACE_SUCCESS);
}


/* --- End of trace.c --- */
