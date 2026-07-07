################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/pH_sensor/pH.c 

OBJS += \
./Application/pH_sensor/pH.o 

C_DEPS += \
./Application/pH_sensor/pH.d 


# Each subdirectory must supply rules for building sources it contributes
Application/pH_sensor/%.o Application/pH_sensor/%.su Application/pH_sensor/%.cyclo: ../Application/pH_sensor/%.c Application/pH_sensor/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DCMWX1ZZABZ0XX -c -I../../../LoRaWAN/App -I../../../LoRaWAN/Target -I../../../Core/Inc -I../../../Utilities/misc -I../../../Utilities/timer -I../../../Utilities/trace/adv_trace -I../../../Utilities/lpm/tiny_lpm -I../../../Utilities/sequencer -I../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../Drivers/BSP/CMWX1ZZABZ_0xx -I../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../Drivers/CMSIS/Include -I../../../Middlewares/Third_Party/SubGHz_Phy -I../../../Middlewares/Third_Party/SubGHz_Phy/sx1276 -I../../../Middlewares/Third_Party/LoRaWAN/Crypto -I../../../Middlewares/Third_Party/LoRaWAN/Mac -I../../../Middlewares/Third_Party/LoRaWAN/Mac/Region -I../../../Middlewares/Third_Party/LoRaWAN/Utilities -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages -I"/home/icfoss/STM32CubeIDE/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/cmwx1zzabz_0xx/Application/do_sensor" -I"/home/icfoss/STM32CubeIDE/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/cmwx1zzabz_0xx/Application/pH_sensor" -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Application-2f-pH_sensor

clean-Application-2f-pH_sensor:
	-$(RM) ./Application/pH_sensor/pH.cyclo ./Application/pH_sensor/pH.d ./Application/pH_sensor/pH.o ./Application/pH_sensor/pH.su

.PHONY: clean-Application-2f-pH_sensor

