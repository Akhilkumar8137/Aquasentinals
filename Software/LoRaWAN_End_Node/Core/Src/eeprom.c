/*
 * eeprom.c
 *
 *  Created on: Feb 20, 2025
 *      Author: zero
 */


#include "eeprom.h"

/*-------------------------------------------------------------------------
  Internal low-level EEPROM write routine.
  This routine writes data byte-by-byte without performing address checks.
-------------------------------------------------------------------------*/
static HAL_StatusTypeDef EEPROM_Write_Internal(uint32_t address, uint8_t *data, uint32_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    HAL_FLASHEx_DATAEEPROM_Unlock();

    for (uint32_t i = 0; i < size; i++) {
        status = HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, address + i, data[i]);
        if (status != HAL_OK) {
            HAL_FLASHEx_DATAEEPROM_Lock();
            return (status);
        }

        if (*(__IO uint8_t*)(address + i) != data[i]) {
            HAL_FLASHEx_DATAEEPROM_Lock();
            return (HAL_ERROR);
        }
    }
    HAL_FLASHEx_DATAEEPROM_Lock();
    return (status);
}

/*-------------------------------------------------------------------------
  Generic EEPROM Write function for User Data.
  This function rejects writes that fall entirely within the reserved fault log region.
-------------------------------------------------------------------------*/
HAL_StatusTypeDef userEEPROM_Write(uint32_t address, uint8_t *data, uint32_t size)
{

    if ((address >= FAULT_LOG_EEPROM_START) && ((address + size - 1) <= FAULT_LOG_EEPROM_END)) {
        return (HAL_ERROR);
    }
    if ((address < DATA_EEPROM_BASE) || ((address + size - 1) > DATA_EEPROM_BANK2_END)) {
        return (HAL_ERROR);
    }
    return (EEPROM_Write_Internal(address, data, size));
}

/*-------------------------------------------------------------------------
  Generic EEPROM Read function.
  This routine is not restricted; caller must supply a valid address.
-------------------------------------------------------------------------*/
void EEPROM_Read(uint32_t address, uint8_t *data, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        data[i] = *(__IO uint8_t*)(address + i);
    }
}

/*-------------------------------------------------------------------------
  User EEPROM Read function for the User region.
  Ensures the requested read is within the User EEPROM area.
-------------------------------------------------------------------------*/
void userEEPROM_Read(uint32_t address, uint8_t *data, uint32_t size)
{
    if ((address < USER_EEPROM_START) || ((address + size - 1) > USER_EEPROM_END)) {
        return;
    }
    EEPROM_Read(address, data, size);
}

/*-------------------------------------------------------------------------
  Retrieve the first free EEPROM address in the User region.
  Assumes an erased EEPROM cell returns 0x00.
-------------------------------------------------------------------------*/
uint32_t userEEPROM_GetFreeStart(void)
{
    uint32_t freeAddr = USER_EEPROM_START;
    uint8_t cell;

    while (freeAddr <= USER_EEPROM_END) {
        cell = *(__IO uint8_t*)freeAddr;
        if (cell == 0x00) {
            return (freeAddr);
        }
        freeAddr++;
    }
    return (USER_EEPROM_END + 1);
}

/*-------------------------------------------------------------------------
  Fault EEPROM Write function – allows writes only within the reserved fault log region.
-------------------------------------------------------------------------*/
HAL_StatusTypeDef FaultEEPROM_Write(uint32_t address, uint8_t *data, uint32_t size)
{
    if ((address < FAULT_LOG_EEPROM_START) || ((address + size - 1) > FAULT_LOG_EEPROM_END)) {
        return (HAL_ERROR);
    }
    return (EEPROM_Write_Internal(address, data, size));
}

/*-------------------------------------------------------------------------
  Fault EEPROM Read function – reads data from the reserved fault log region.
-------------------------------------------------------------------------*/
void FaultEEPROM_Read(uint32_t address, uint8_t *data, uint32_t size)
{
    if ((address < FAULT_LOG_EEPROM_START) || ((address + size - 1) > FAULT_LOG_EEPROM_END)) {
        return;
    }
    EEPROM_Read(address, data, size);
}

/*-------------------------------------------------------------------------
  Clears the fault log region by writing zeros.
-------------------------------------------------------------------------*/
HAL_StatusTypeDef ClearFaultLog(void)
{
    uint8_t clearBuffer[sizeof(FaultLog_t)] = {0};
    return (FaultEEPROM_Write(FAULT_LOG_EEPROM_START, clearBuffer, sizeof(FaultLog_t)));
}
