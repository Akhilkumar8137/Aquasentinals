/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    lora_app.c
  * @author  MCD Application Team
  * @brief   Application of the LRWAN Middleware
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "Region.h" /* Needed for LORAWAN_DEFAULT_DATA_RATE */
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "utilities_def.h"
#include "lora_app_version.h"
#include "lorawan_version.h"
#include "subghz_phy_version.h"
#include "lora_info.h"
#include "LmHandler.h"
#include "stm32_lpm.h"
#include "adc_if.h"
#include "sys_conf.h"
#include "CayenneLpp.h"
#include "eeprom.h"
#include "main.h"
#include "do_sensor.h"
#include "pH.h"
#include "timer6.h"
#include "string.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */
/* USER CODE BEGIN EV */
float temperature = 0.0f;
float voltage     = 0.0f;
float do_value    = 0.0f;
/* USER CODE END EV */


/* Private typedef -----------------------------------------------------------*/
/**
  * @brief LoRa State Machine states
  */
typedef enum TxEventType_e
{
  /**
    * @brief Appdata Transmission issue based on timer every TxDutyCycleTime
    */
  TX_ON_TIMER,
  /**
    * @brief Appdata Transmission external event plugged on OnSendEvent( )
    */
  TX_ON_EVENT
  /* USER CODE BEGIN TxEventType_t */

  /* USER CODE END TxEventType_t */
} TxEventType_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  LoRa End Node send request
  */
static void SendTxData(void);

/**
  * @brief  TX timer callback function
  * @param  context ptr of timer context
  */
static void OnTxTimerEvent(void *context);

/**
  * @brief  join event callback function
  * @param  joinParams status of join
  */
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);

/**
  * @brief  tx event callback function
  * @param  params status of last Tx
  */
static void OnTxData(LmHandlerTxParams_t *params);

/**
  * @brief callback when LoRa application has received a frame
  * @param appData data received in the last Rx
  * @param params status of last Rx
  */
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

/*!
 * Will be called each time a Radio IRQ is handled by the MAC layer
 *
 */
static void OnMacProcessNotify(void);

/**
  * @brief  send Fault log data incase of unpredictable error occurs.
  */
static void sendFaultData(void);


/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private variables ---------------------------------------------------------*/
static ActivationType_t ActivationType = LORAWAN_DEFAULT_ACTIVATION_TYPE;

/**
  * @brief LoRaWAN handler Callbacks
  */
static LmHandlerCallbacks_t LmHandlerCallbacks =
{
  .GetBatteryLevel =           GetBatteryLevel,
  .GetTemperature =            GetTemperatureLevel,
  .GetUniqueId =               GetUniqueId,
  .GetDevAddr =                GetDevAddr,
  .OnMacProcess =              OnMacProcessNotify,
  .OnJoinRequest =             OnJoinRequest,
  .OnTxData =                  OnTxData,
  .OnRxData =                  OnRxData
};

/**
  * @brief LoRaWAN handler parameters
  */
static LmHandlerParams_t LmHandlerParams =
{
  .ActiveRegion =             ACTIVE_REGION,
  .DefaultClass =             LORAWAN_DEFAULT_CLASS,
  .AdrEnable =                LORAWAN_ADR_STATE,
  .TxDatarate =               LORAWAN_DEFAULT_DATA_RATE,
  .PingPeriodicity =          LORAWAN_DEFAULT_PING_SLOT_PERIODICITY
};

/**
  * @brief Type of Event to generate application Tx
  */
static TxEventType_t EventType = TX_ON_TIMER;

/**
  * @brief Timer to handle the application Tx
  */
static UTIL_TIMER_Object_t TxTimer;

/* USER CODE BEGIN PV */
/**
  * @brief User application buffer
  */
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/**
  * @brief User application data structure
  */
static LmHandlerAppData_t AppData = { 0, 0, AppDataBuffer };


/* USER CODE END PV */

/* Exported functions ---------------------------------------------------------*/
/* USER CODE BEGIN EF */

/* USER CODE END EF */

void LoRaWAN_Init(void)
{
  /* USER CODE BEGIN LoRaWAN_Init_1 */

  /* Get LoRa APP version*/
  APP_LOG(TS_OFF, VLEVEL_M, "APP_VERSION:        V%X.%X.%X\r\n",
          (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_MAIN_SHIFT),
          (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB1_SHIFT),
          (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB2_SHIFT));

  /* Get MW LoraWAN info */
  APP_LOG(TS_OFF, VLEVEL_M, "MW_LORAWAN_VERSION: V%X.%X.%X\r\n",
          (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_MAIN_SHIFT),
          (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB1_SHIFT),
          (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB2_SHIFT));

  /* Get MW SubGhz_Phy info */
  APP_LOG(TS_OFF, VLEVEL_M, "MW_RADIO_VERSION:   V%X.%X.%X\r\n",
          (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_MAIN_SHIFT),
          (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB1_SHIFT),
          (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB2_SHIFT));

 /*Register Tasks */

  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), UTIL_SEQ_RFU, SendTxData);

  /* Init Info table used by LmHandler*/
  LoraInfo_Init();

  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  LmHandlerConfigure(&LmHandlerParams);

  LmHandlerJoin(ActivationType);



  if (EventType == TX_ON_TIMER)
  {
    /* send every time timer elapses */
    UTIL_TIMER_Create(&TxTimer,  0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
    UTIL_TIMER_SetPeriod(&TxTimer,  APP_TX_DUTYCYCLE);
    UTIL_TIMER_Start(&TxTimer);
  }
  else
  {
	  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
  }

}

/* GPIO External callback */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	switch(GPIO_Pin)
	{
	case GPIO_PIN_2:
		UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);
		break;
	default:
		APP_PRINTF("Unknown_Interrupt\r\n");
		break;
	}
}

/* USER CODE END PB_Callbacks */

/* Private functions ---------------------------------------------------------*/
/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  /* USER CODE BEGIN OnRxData_1 */
  if ((appData != NULL) || (params != NULL))
  {

    static const char *slotStrings[] = { "1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot" };

    APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ========== MCPS-Indication ==========\r\n");
    APP_LOG(TS_OFF, VLEVEL_H, "###### D/L FRAME:%04d | SLOT:%s | PORT:%d | DR:%d | RSSI:%d | SNR:%d\r\n",
            params->DownlinkCounter, slotStrings[params->RxSlot], appData->Port, params->Datarate, params->Rssi, params->Snr);
    switch (appData->Port)
    {
      case LORAWAN_SWITCH_CLASS_PORT:
        /*this port switches the class*/
        if (appData->BufferSize == 1)
        {
          switch (appData->Buffer[0])
          {
            case 0:
            {
              LmHandlerRequestClass(CLASS_A);
              break;
            }
            case 1:
            {
              LmHandlerRequestClass(CLASS_B);
              break;
            }
            case 2:
            {
              LmHandlerRequestClass(CLASS_C);
              break;
            }
            default:
              break;
          }
        }
        break;
      default:
        break;
    }
  }
  /* USER CODE END OnRxData_1 */
}
static void sendFaultData(void)
{
	FaultLog_t faultLog;
	FaultEEPROM_Read(FAULT_LOG_EEPROM_START, (uint8_t *)&faultLog, sizeof(FaultLog_t));
	if (faultLog.faultFlag == FAULT_OCCURRED_FLAG) {
	AppData.Port = 10;
	uint32_t buffer_index = 0;
	UTIL_TIMER_Time_t nextTxIn = 0;

//	AppData.Buffer[buffer_index++] = (faultLog.faultFlag >> 24) & 0xFF;
//	AppData.Buffer[buffer_index++] = (faultLog.faultFlag >> 16) & 0xFF;
//	AppData.Buffer[buffer_index++] = (faultLog.faultFlag >> 8) & 0xFF;
//	AppData.Buffer[buffer_index++] = faultLog.faultFlag & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.faultPC >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.faultPC >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.faultPC >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.faultPC & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.stacked_xPSR >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.stacked_xPSR >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.stacked_xPSR >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.stacked_xPSR & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.stacked_LR >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.stacked_LR >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.stacked_LR >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.stacked_LR & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.stacked_r12 >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.stacked_r12 >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.stacked_r12 >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.stacked_r12 & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.current_IPSR >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.current_IPSR >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.current_IPSR >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.current_IPSR & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.current_APSR >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.current_APSR >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.current_APSR >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.current_APSR & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.current_xPSR >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.current_xPSR >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.current_xPSR >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.current_xPSR & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.PSP >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.PSP >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.PSP >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.PSP & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.CONTROL >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.CONTROL >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.CONTROL >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.CONTROL & 0xFF;

	AppData.Buffer[buffer_index++] = (faultLog.MSP >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.MSP >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.MSP >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.MSP & 0xFF;


	AppData.Buffer[buffer_index++] = (faultLog.faultCode >> 24) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.faultCode >> 16) & 0xFF;
	AppData.Buffer[buffer_index++] = (faultLog.faultCode >> 8) & 0xFF;
	AppData.Buffer[buffer_index++] = faultLog.faultCode & 0xFF;



	  if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAMAC_HANDLER_CONFIRMED_MSG, &nextTxIn, false))
	  {
	    APP_LOG(TS_ON, VLEVEL_L, "SEND Fault REQUEST\r\n");
	  }
	  ClearFaultLog();
	}
}

static void SendTxData(void)
{
  /* USER CODE BEGIN SendTxData_1 */

  UTIL_TIMER_Time_t nextTxIn = 0;

#ifdef CAYENNE_LPP
  uint8_t channel = 0;
#else
/*sensor value declerations */
  uint32_t i = 0;
  float temperature = Sensors_ReadTemperature();
  float voltage     = Sensors_ReadVoltage();
  float do_value    = Sensors_ReadDO();
  pH_Data_t sensorData;
  Get_pH_Measurement(&sensorData);
  uint16_t voltage_i = (uint16_t)(sensorData.voltage* 1000);
  uint16_t pH_i   = (uint16_t)(sensorData.ph_val * 100);

  int16_t  temp_i = (int16_t)(temperature * 100);
  uint16_t volt_i = (uint16_t)(voltage * 1000);
  uint16_t do_i   = (uint16_t)(do_value * 100);


#endif /* CAYENNE_LPP */



	AppData.Port = LORAWAN_USER_APP_PORT;

	AppData.Buffer[i++] = temp_i & 0xFF;
	AppData.Buffer[i++] = (temp_i >> 8) & 0xFF;

	AppData.Buffer[i++] = volt_i & 0xFF;
	AppData.Buffer[i++] = (volt_i >> 8) & 0xFF;


	AppData.Buffer[i++] = do_i & 0xFF;
	AppData.Buffer[i++] = (do_i >> 8) & 0xFF;

	AppData.Buffer[i++] = voltage_i & 0xFF;
	AppData.Buffer[i++] = (voltage_i >> 8) & 0xFF;


	AppData.Buffer[i++] = pH_i & 0xFF;
	AppData.Buffer[i++] = (pH_i >> 8) & 0xFF;



#ifdef CAYENNE_LPP
  CayenneLppReset();
  CayenneLppAddBarometricPressure(channel++, pressure);
  CayenneLppAddTemperature(channel++, temperature);
  CayenneLppAddRelativeHumidity(channel++, (uint16_t)(sensor_data.humidity));

  if ((LmHandlerParams.ActiveRegion != LORAMAC_REGION_US915) && (LmHandlerParams.ActiveRegion != LORAMAC_REGION_AU915)
      && (LmHandlerParams.ActiveRegion != LORAMAC_REGION_AS923))
  {
    CayenneLppAddDigitalInput(channel++, GetBatteryLevel());
    CayenneLppAddDigitalOutput(channel++, AppLedStateOn);
  }

  CayenneLppCopy(AppData.Buffer);
  AppData.BufferSize = CayenneLppGetSize();
#else  /* not CAYENNE_LPP */

  AppData.BufferSize = i;
#endif /* CAYENNE_LPP */

  if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false))
  {
    APP_LOG(TS_ON, VLEVEL_L, "SEND REQUEST\r\n");
  }
  else if (nextTxIn > 0)
  {
    APP_LOG(TS_ON, VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n", (nextTxIn / 1000));
  }

  /* USER CODE END SendTxData_1 */
}

static void OnTxTimerEvent(void *context)
{
  /* USER CODE BEGIN OnTxTimerEvent_1 */

  /* USER CODE END OnTxTimerEvent_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);

  /*Wait for next tx slot*/
  UTIL_TIMER_Start(&TxTimer);
  /* USER CODE BEGIN OnTxTimerEvent_2 */

  /* USER CODE END OnTxTimerEvent_2 */
}



static void OnTxData(LmHandlerTxParams_t *params)
{
  /* USER CODE BEGIN OnTxData_1 */
  if ((params != NULL))
  {
    /* Process Tx event only if its a mcps response to prevent some internal events (mlme) */
    if (params->IsMcpsConfirm != 0)
    {

      APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ========== MCPS-Confirm =============\r\n");
      APP_LOG(TS_OFF, VLEVEL_H, "###### U/L FRAME:%04d | PORT:%d | DR:%d | PWR:%d", params->UplinkCounter,
              params->AppData.Port, params->Datarate, params->TxPower);

      APP_LOG(TS_OFF, VLEVEL_H, " | MSG TYPE:");
      if (params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG)
      {
        APP_LOG(TS_OFF, VLEVEL_H, "CONFIRMED [%s]\r\n", (params->AckReceived != 0) ? "ACK" : "NACK");
      }
      else
      {
        APP_LOG(TS_OFF, VLEVEL_H, "UNCONFIRMED\r\n");
      }
    }
  }
  /* USER CODE END OnTxData_1 */
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  /* USER CODE BEGIN OnJoinRequest_1 */
  if (joinParams != NULL)
  {
    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS)
    {

      APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### = JOINED = ");
      if (joinParams->Mode == ACTIVATION_TYPE_ABP)
      {
        APP_LOG(TS_OFF, VLEVEL_M, "ABP ======================\r\n");

        sendFaultData();

        /**
         *  Send the initial data once peripheral initialization is completed
         */
        	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);
      }
      else
      {
        APP_LOG(TS_OFF, VLEVEL_M, "OTAA =====================\r\n");

        sendFaultData();
        /**
         *  Send the initial data once peripheral initialization is completed
         */
        	UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);
      }
    }
    else
    {
      APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### = JOIN FAILED\r\n");
    }
  }
  /* USER CODE END OnJoinRequest_1 */
}

static void OnMacProcessNotify(void)
{
  /* USER CODE BEGIN OnMacProcessNotify_1 */

  /* USER CODE END OnMacProcessNotify_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);

  /* USER CODE BEGIN OnMacProcessNotify_2 */

  /* USER CODE END OnMacProcessNotify_2 */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
