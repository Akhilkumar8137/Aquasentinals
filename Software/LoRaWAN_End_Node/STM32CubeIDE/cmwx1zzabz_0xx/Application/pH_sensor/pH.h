#ifndef __PH_H__
#define __PH_H__

#include "main.h"

/* --- Hardware Connections --- */
#define DS18B20_PORT  GPIOB
#define DS18B20_PIN   GPIO_PIN_6
 // PA0 (A0 on header)

/* --- Calibration Constants --- */
#define PH7_VOLTAGE   1525.0f   // mV at pH 7
#define PH4_VOLTAGE   2050.0f   // mV at pH 9.2

/* --- Data Structure --- */
typedef struct {
    float temperature;
    float voltage;
    float ph_val;
} pH_Data_t;

/* --- Function Prototypes --- */
void Get_pH_Measurement(pH_Data_t *data);
//uint32_t Read_ADC_Avg(uint32_t channel);

#endif /* __PH_H__ */
