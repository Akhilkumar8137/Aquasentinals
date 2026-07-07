################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/Raingauge_Station/raingauge_station.c 

OBJS += \
./Application/Raingauge_Station/raingauge_station.o 

C_DEPS += \
./Application/Raingauge_Station/raingauge_station.d 


# Each subdirectory must supply rules for building sources it contributes
Application/Raingauge_Station/%.o Application/Raingauge_Station/%.su Application/Raingauge_Station/%.cyclo: ../Application/Raingauge_Station/%.c Application/Raingauge_Station/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DCMWX1ZZABZ0XX -DRAIN -c -I../../../LoRaWAN/App -I"/home/shafeekpm/Videos/gitlabtest/Software/STM32CubeIDE/cmwx1zzabz_0xx/Application/Raingauge_Station" -I../../../LoRaWAN/Target -I../../../Core/Inc -I../../../Utilities/misc -I../../../Utilities/timer -I../../../Utilities/trace/adv_trace -I../../../Utilities/lpm/tiny_lpm -I../../../Utilities/sequencer -I../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../Drivers/BSP/CMWX1ZZABZ_0xx -I../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../Drivers/CMSIS/Include -I../../../Middlewares/Third_Party/SubGHz_Phy -I../../../Middlewares/Third_Party/SubGHz_Phy/sx1276 -I../../../Middlewares/Third_Party/LoRaWAN/Crypto -I../../../Middlewares/Third_Party/LoRaWAN/Mac -I../../../Middlewares/Third_Party/LoRaWAN/Mac/Region -I../../../Middlewares/Third_Party/LoRaWAN/Utilities -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler -I../../../Drivers/BSP/IKS01A2 -I../../../Drivers/BSP/Components/Common -I../../../Drivers/BSP/Components/hts221 -I../../../Drivers/BSP/Components/lps22hb -I../../../Drivers/BSP/Components/lsm6dsl -I../../../Drivers/BSP/Components/lsm303agr -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Application-2f-Raingauge_Station

clean-Application-2f-Raingauge_Station:
	-$(RM) ./Application/Raingauge_Station/raingauge_station.cyclo ./Application/Raingauge_Station/raingauge_station.d ./Application/Raingauge_Station/raingauge_station.o ./Application/Raingauge_Station/raingauge_station.su

.PHONY: clean-Application-2f-Raingauge_Station

