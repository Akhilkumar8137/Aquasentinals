#include "do_sensor.h"
#include "adc_if.h"
#include "timer6.h"
#include "stm32l0xx_hal.h"
#include "pH.h"

#define VREF        3.3f
#define ADC_MAX     4095.0f

#define CAL1_V      0.867f
#define CAL1_T      25.0f

#define DS18B20_PORT GPIOB
#define DS18B20_PIN  GPIO_PIN_6

#define DO_ADC_CHANNEL ADC_CHANNEL_5
#define PH_ADC_CHANNEL ADC_CHANNEL_0

#define DO_MOSFET_PORT   GPIOB
#define DO_MOSFET_PIN    GPIO_PIN_2   // PB2

#define PH_MOSFET_PORT   GPIOB
#define PH_MOSFET_PIN    GPIO_PIN_7   // PB7


extern TIM_HandleTypeDef htim6;



static void delay_us(uint16_t us);
static uint32_t Read_ADC_Avg(uint32_t channel);
static void DS18B20_SetOutput(void);
static void DS18B20_SetInput(void);

static uint8_t DS18B20_Reset(void);
static void DS18B20_Write(uint8_t data);
static uint8_t DS18B20_Read(void);

/* GPIO Init  */
static void DS18B20_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin  = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}


static void MOSFET_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = DO_MOSFET_PIN;
    HAL_GPIO_Init(DO_MOSFET_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = PH_MOSFET_PIN;
    HAL_GPIO_Init(PH_MOSFET_PORT, &GPIO_InitStruct);

    /* Start with DO OFF, pH ON */
    HAL_GPIO_WritePin(DO_MOSFET_PORT, DO_MOSFET_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PH_MOSFET_PORT, PH_MOSFET_PIN, GPIO_PIN_SET);
}



/*  DSB18B20 SETUP */
static void DS18B20_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = DS18B20_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

static void DS18B20_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = DS18B20_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DS18B20_PORT, &GPIO_InitStruct);
}

/* SENSORS READING */
static void DO_Sensor_ON(void)
{
    HAL_GPIO_WritePin(DO_MOSFET_PORT, DO_MOSFET_PIN, GPIO_PIN_SET);
    HAL_Delay(60000);   // warm-up
}

static void DO_Sensor_OFF(void)
{
    HAL_GPIO_WritePin(DO_MOSFET_PORT, DO_MOSFET_PIN, GPIO_PIN_RESET);
    HAL_Delay(60000);
}




float Sensors_ReadTemperature(void)
{
    if (!DS18B20_Reset())
        return -1000.0f;

    DS18B20_Write(0xCC);
    DS18B20_Write(0x44);
    HAL_Delay(750);

    if (!DS18B20_Reset())
        return -1000.0f;

    DS18B20_Write(0xCC);
    DS18B20_Write(0xBE);

    uint8_t lsb = DS18B20_Read();
    uint8_t msb = DS18B20_Read();

    int16_t raw = (msb << 8) | lsb;
    return raw / 16.0f;
}
static uint32_t Read_ADC_Avg(uint32_t channel)
{
    uint64_t sum = 0;

    for (int i = 0; i < 32; i++)
    {
        sum += ADC_ReadChannels(channel);
        HAL_Delay(5);
    }

    return (uint32_t)(sum / 32);
}

void Get_pH_Measurement(pH_Data_t *data)
{
	HAL_Delay(5000);

    data->temperature = Sensors_ReadTemperature();

    uint32_t adc_raw = Read_ADC_Avg(PH_ADC_CHANNEL);
    float voltage_v  = (adc_raw * VREF) / ADC_MAX;
    float voltage_mv = voltage_v * 1000.0f;
    data->voltage = voltage_v;

    float slope = (4.0f - 7.0f) / (PH4_VOLTAGE - PH7_VOLTAGE);
    float ph = 7.0f + slope * (voltage_mv - PH7_VOLTAGE);

    ph += (data->temperature - 25.0f) * 0.015f;

    if (ph < 0)  ph = 0;
    if (ph > 14) ph = 14;

    data->ph_val = ph;


}




float Sensors_ReadVoltage(void)
{
    uint16_t adc = ADC_ReadChannels(DO_ADC_CHANNEL);
    HAL_Delay(400);
    return (adc * VREF ) / ADC_MAX;
}

float Sensors_ReadDO(void)
{
    DO_Sensor_ON();

    float voltage     = Sensors_ReadVoltage();
    float temperature = Sensors_ReadTemperature();


    if (temperature < 0)  temperature = 0;
    if (temperature > 40) temperature = 40;

    uint8_t temp_i = (uint8_t)(temperature + 0.5f);

    float do_mgL = (voltage * DO_Table[temp_i]) / (CAL1_V * 1000.0f);
    if (do_mgL < 0) do_mgL = 0;
    DO_Sensor_OFF();
    return do_mgL;


}


/*DELAY*/

static void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim6, 0);
    while (__HAL_TIM_GET_COUNTER(&htim6) < us);
}

/* DS18B20 */

static uint8_t DS18B20_Reset(void)
{
    DS18B20_SetOutput();
    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
    delay_us(480);

    DS18B20_SetInput();
    delay_us(70);

    uint8_t presence =
        (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN) == GPIO_PIN_RESET);

    delay_us(410);
    return presence;
}

static void DS18B20_Write(uint8_t data)
{
    for (int i = 0; i < 8; i++)
    {
        DS18B20_SetOutput();
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);

        if (data & 0x01)
        {
            delay_us(5);
            DS18B20_SetInput();
            delay_us(55);
        }
        else
        {
            delay_us(60);
            DS18B20_SetInput();
        }
        data >>= 1;
    }
}

static uint8_t DS18B20_Read(void)
{
    uint8_t value = 0;

    for (int i = 0; i < 8; i++)
    {
        DS18B20_SetOutput();
        HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET);
        delay_us(3);

        DS18B20_SetInput();
        delay_us(10);

        if (HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN))
            value |= (1 << i);

        delay_us(53);
    }
    return value;
}

/* SENSORS Init*/

void Sensors_Init(void)
{
	DS18B20_GPIO_Init();
    MX_TIM6_Init();
    HAL_TIM_Base_Start(&htim6);
    MOSFET_GPIO_Init();
}


