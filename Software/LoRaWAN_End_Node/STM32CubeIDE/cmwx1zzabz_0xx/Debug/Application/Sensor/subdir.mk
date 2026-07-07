################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/Sensor/i2c.c 

OBJS += \
./Application/Sensor/i2c.o 

C_DEPS += \
./Application/Sensor/i2c.d 


# Each subdirectory must supply rules for building sources it contributes
Application/Sensor/%.o Application/Sensor/%.su Application/Sensor/%.cyclo: ../Application/Sensor/%.c Application/Sensor/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DCMWX1ZZABZ0XX -c -I../../../LoRaWAN/App -I../../../LoRaWAN/Target -I../../../Core/Inc -I../../../Utilities/misc -I../../../Utilities/timer -I../../../Utilities/trace/adv_trace -I../../../Utilities/lpm/tiny_lpm -I../../../Utilities/sequencer -I../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../Drivers/BSP/CMWX1ZZABZ_0xx -I../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../Drivers/CMSIS/Include -I../../../Middlewares/Third_Party/SubGHz_Phy -I../../../Middlewares/Third_Party/SubGHz_Phy/sx1276 -I../../../Middlewares/Third_Party/LoRaWAN/Crypto -I../../../Middlewares/Third_Party/LoRaWAN/Mac -I../../../Middlewares/Third_Party/LoRaWAN/Mac/Region -I../../../Middlewares/Third_Party/LoRaWAN/Utilities -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler -I../../../Drivers/BSP/IKS01A2 -I../../../Drivers/BSP/Components/Common -I../../../Drivers/BSP/Components/hts221 -I../../../Drivers/BSP/Components/lps22hb -I../../../Drivers/BSP/Components/lsm6dsl -I../../../Drivers/BSP/Components/lsm303agr -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages -I"/home/shafeekpm/Music/july6th_folder/stm32_stack_analysis/LoRaWAN_End_Node/STM32CubeIDE/cmwx1zzabz_0xx/Application/Sensor" -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Application-2f-Sensor

clean-Application-2f-Sensor:
	-$(RM) ./Application/Sensor/i2c.cyclo ./Application/Sensor/i2c.d ./Application/Sensor/i2c.o ./Application/Sensor/i2c.su

.PHONY: clean-Application-2f-Sensor

