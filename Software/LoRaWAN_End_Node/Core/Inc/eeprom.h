/*
 * eeprom.h
 *
 *  Created on: Feb 20, 2025
 *      Author: zero
 */

#ifndef APPLICATION_EEPROM_INC_EEPROM_H_
#define APPLICATION_EEPROM_INC_EEPROM_H_

#include "main.h"

typedef enum {
	HARDFAULT_CODE = 0x01,
	NMIFAULT_CODE,
	MEMORYMANAGEFAULT_CODE,
	BUSFAULT_CODE,
	USAGEFAULT_CODE,
	ERROR_HANDLER
}eepromFaultCode_t;

#define DATA_EEPROM_SIZE       (DATA_EEPROM_BANK2_END - DATA_EEPROM_BASE + 1)

/* Reserve the first 50 bytes for fault logging */
#define FAULT_LOG_SIZE         (50U)
#define FAULT_LOG_EEPROM_START DATA_EEPROM_BASE
#define FAULT_LOG_EEPROM_END   (FAULT_LOG_EEPROM_START + FAULT_LOG_SIZE - 1)

#define USER_EEPROM_START   (FAULT_LOG_EEPROM_END + 1)
#define USER_EEPROM_END     DATA_EEPROM_BANK2_END
#define USER_EEPROM_SIZE    (GENERAL_EEPROM_END - GENERAL_EEPROM_START + 1)

typedef struct {
    uint32_t faultFlag;
    uint32_t faultPC;
    uint32_t stacked_xPSR;
    uint32_t stacked_LR;
    uint32_t stacked_r12;
    uint32_t current_IPSR;
    uint32_t current_APSR;
    uint32_t current_xPSR;
    uint32_t PSP;
    uint32_t CONTROL;
    uint32_t MSP;
    uint32_t faultCode;
} FaultLog_t;

#define FAULT_OCCURRED_FLAG  0xA5A5A5A5

/*-----------------------------------------------------------------------
  USER EEPROM API – General data functions.
------------------------------------------------------------------------*/
/**
  * @brief  Write data to the general EEPROM area.
  *         This function rejects writes that would fall into the reserved fault log region.
  * @param  address: Starting EEPROM address (must be within the general area).
  * @param  data: Pointer to the data buffer.
  * @param  size: Number of bytes to write.
  * @retval HAL_StatusTypeDef status.
  */
HAL_StatusTypeDef userEEPROM_Write(uint32_t address, uint8_t *data, uint32_t size);

/**
  * @brief  Read data from the user EEPROM area.
  * @param  address: Starting EEPROM address (must be within the general area).
  * @param  data: Pointer to the buffer to store read data.
  * @param  size: Number of bytes to read.
  * @retval None.
  */
void userEEPROM_Read(uint32_t address, uint8_t *data, uint32_t size);

/**
  * @brief  Retrieve the first free EEPROM address within the user area.
  *         Scans from USER_EEPROM_START to USER_EEPROM_END and returns the first
  *         address containing 0xFF (assuming erased EEPROM returns 0xFF).
  * @retval uint32_t The first free address; if full, returns USER_EEPROM_END + 1.
  */
uint32_t userEEPROM_GetFreeStart(void);

/*-----------------------------------------------------------------------
  EEPROM API – Fault Log functions.
------------------------------------------------------------------------*/
/**
  * @brief  Write data to the reserved fault log EEPROM area.
  *         Allows writes only within the fault log region.
  * @param  address: Starting EEPROM address (must be within fault log region).
  * @param  data: Pointer to the fault log data.
  * @param  size: Number of bytes to write.
  * @retval HAL_StatusTypeDef status.
  */
HAL_StatusTypeDef FaultEEPROM_Write(uint32_t address, uint8_t *data, uint32_t size);

/**
  * @brief  Read data from the fault log EEPROM area.
  * @param  address: Starting EEPROM address (must be within fault log region).
  * @param  data: Pointer to the buffer to store fault log data.
  * @param  size: Number of bytes to read.
  * @retval None.
  */
void FaultEEPROM_Read(uint32_t address, uint8_t *data, uint32_t size);

/**
  * @brief  Clear the fault log region in EEPROM by writing zeros.
  * @retval HAL_StatusTypeDef status.
  */
HAL_StatusTypeDef ClearFaultLog(void);

#endif /* APPLICATION_EEPROM_INC_EEPROM_H_ */
