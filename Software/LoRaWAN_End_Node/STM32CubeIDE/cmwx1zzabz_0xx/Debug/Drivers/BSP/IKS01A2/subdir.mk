################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/zero/Projects/STM/Experiments/c1-dev_firmware_updation/LoRaWAN_End_Node/Drivers/BSP/IKS01A2/iks01a2_env_sensors.c \
/home/zero/Projects/STM/Experiments/c1-dev_firmware_updation/LoRaWAN_End_Node/Drivers/BSP/IKS01A2/iks01a2_env_sensors_ex.c \
/home/zero/Projects/STM/Experiments/c1-dev_firmware_updation/LoRaWAN_End_Node/Drivers/BSP/IKS01A2/iks01a2_motion_sensors.c \
/home/zero/Projects/STM/Experiments/c1-dev_firmware_updation/LoRaWAN_End_Node/Drivers/BSP/IKS01A2/iks01a2_motion_sensors_ex.c 

OBJS += \
./Drivers/BSP/IKS01A2/iks01a2_env_sensors.o \
./Drivers/BSP/IKS01A2/iks01a2_env_sensors_ex.o \
./Drivers/BSP/IKS01A2/iks01a2_motion_sensors.o \
./Drivers/BSP/IKS01A2/iks01a2_motion_sensors_ex.o 

C_DEPS += \
./Drivers/BSP/IKS01A2/iks01a2_env_sensors.d \
./Drivers/BSP/IKS01A2/iks01a2_env_sensors_ex.d \
./Drivers/BSP/IKS01A2/iks01a2_motion_sensors.d \
./Drivers/BSP/IKS01A2/iks01a2_motion_sensors_ex.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/IKS01A2/iks01a2_env_sensors.o: /home/zero/Projects/STM/Experiments/c1-dev_firmware_updation/LoRaWAN_End_Node/Drivers/BSP/IKS01A2/iks01a2_env_sensors.c Drivers/BSP/IKS01A2/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DCMWX1ZZABZ0XX -c -I../../../LoRaWAN/App -I../../../LoRaWAN/Target -I../../../Core/Inc -I../../../Utilities/misc -I../../../Utilities/timer -I../../../Utilities/trace/adv_trace -I../../../Utilities/lpm/tiny_lpm -I../../../Utilities/sequencer -I../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../Drivers/BSP/CMWX1ZZABZ_0xx -I../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../Drivers/CMSIS/Include -I../../../Middlewares/Third_Party/SubGHz_Phy -I../../../Middlewares/Third_Party/SubGHz_Phy/sx1276 -I../../../Middlewares/Third_Party/LoRaWAN/Crypto -I../../../Middlewares/Third_Party/LoRaWAN/Mac -I../../../Middlewares/Third_Party/LoRaWAN/Mac/Region -I../../../Middlewares/Third_Party/LoRaWAN/Utilities -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Drivers/BSP/IKS01A2/iks01a2_env_sensors_ex.o: /home/zero/Projects/STM/Experiments/c1-dev_firmware_updation/LoRaWAN_End_Node/Drivers/BSP/IKS01A2/iks01a2_env_sensors_ex.c Drivers/BSP/IKS01A2/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DCMWX1ZZABZ0XX -c -I../../../LoRaWAN/App -I../../../LoRaWAN/Target -I../../../Core/Inc -I../../../Utilities/misc -I../../../Utilities/timer -I../../../Utilities/trace/adv_trace -I../../../Utilities/lpm/tiny_lpm -I../../../Utilities/sequencer -I../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../Drivers/BSP/CMWX1ZZABZ_0xx -I../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../Drivers/CMSIS/Include -I../../../Middlewares/Third_Party/SubGHz_Phy -I../../../Middlewares/Third_Party/SubGHz_Phy/sx1276 -I../../../Middlewares/Third_Party/LoRaWAN/Crypto -I../../../Middlewares/Third_Party/LoRaWAN/Mac -I../../../Middlewares/Third_Party/LoRaWAN/Mac/Region -I../../../Middlewares/Third_Party/LoRaWAN/Utilities -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Drivers/BSP/IKS01A2/iks01a2_motion_sensors.o: /home/zero/Projects/STM/Experiments/c1-dev_firmware_updation/LoRaWAN_End_Node/Drivers/BSP/IKS01A2/iks01a2_motion_sensors.c Drivers/BSP/IKS01A2/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DCMWX1ZZABZ0XX -c -I../../../LoRaWAN/App -I../../../LoRaWAN/Target -I../../../Core/Inc -I../../../Utilities/misc -I../../../Utilities/timer -I../../../Utilities/trace/adv_trace -I../../../Utilities/lpm/tiny_lpm -I../../../Utilities/sequencer -I../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../Drivers/BSP/CMWX1ZZABZ_0xx -I../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../Drivers/CMSIS/Include -I../../../Middlewares/Third_Party/SubGHz_Phy -I../../../Middlewares/Third_Party/SubGHz_Phy/sx1276 -I../../../Middlewares/Third_Party/LoRaWAN/Crypto -I../../../Middlewares/Third_Party/LoRaWAN/Mac -I../../../Middlewares/Third_Party/LoRaWAN/Mac/Region -I../../../Middlewares/Third_Party/LoRaWAN/Utilities -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Drivers/BSP/IKS01A2/iks01a2_motion_sensors_ex.o: /home/zero/Projects/STM/Experiments/c1-dev_firmware_updation/LoRaWAN_End_Node/Drivers/BSP/IKS01A2/iks01a2_motion_sensors_ex.c Drivers/BSP/IKS01A2/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DCMWX1ZZABZ0XX -c -I../../../LoRaWAN/App -I../../../LoRaWAN/Target -I../../../Core/Inc -I../../../Utilities/misc -I../../../Utilities/timer -I../../../Utilities/trace/adv_trace -I../../../Utilities/lpm/tiny_lpm -I../../../Utilities/sequencer -I../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../Drivers/BSP/CMWX1ZZABZ_0xx -I../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../Drivers/CMSIS/Include -I../../../Middlewares/Third_Party/SubGHz_Phy -I../../../Middlewares/Third_Party/SubGHz_Phy/sx1276 -I../../../Middlewares/Third_Party/LoRaWAN/Crypto -I../../../Middlewares/Third_Party/LoRaWAN/Mac -I../../../Middlewares/Third_Party/LoRaWAN/Mac/Region -I../../../Middlewares/Third_Party/LoRaWAN/Utilities -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-IKS01A2

clean-Drivers-2f-BSP-2f-IKS01A2:
	-$(RM) ./Drivers/BSP/IKS01A2/iks01a2_env_sensors.cyclo ./Drivers/BSP/IKS01A2/iks01a2_env_sensors.d ./Drivers/BSP/IKS01A2/iks01a2_env_sensors.o ./Drivers/BSP/IKS01A2/iks01a2_env_sensors.su ./Drivers/BSP/IKS01A2/iks01a2_env_sensors_ex.cyclo ./Drivers/BSP/IKS01A2/iks01a2_env_sensors_ex.d ./Drivers/BSP/IKS01A2/iks01a2_env_sensors_ex.o ./Drivers/BSP/IKS01A2/iks01a2_env_sensors_ex.su ./Drivers/BSP/IKS01A2/iks01a2_motion_sensors.cyclo ./Drivers/BSP/IKS01A2/iks01a2_motion_sensors.d ./Drivers/BSP/IKS01A2/iks01a2_motion_sensors.o ./Drivers/BSP/IKS01A2/iks01a2_motion_sensors.su ./Drivers/BSP/IKS01A2/iks01a2_motion_sensors_ex.cyclo ./Drivers/BSP/IKS01A2/iks01a2_motion_sensors_ex.d ./Drivers/BSP/IKS01A2/iks01a2_motion_sensors_ex.o ./Drivers/BSP/IKS01A2/iks01a2_motion_sensors_ex.su

.PHONY: clean-Drivers-2f-BSP-2f-IKS01A2

