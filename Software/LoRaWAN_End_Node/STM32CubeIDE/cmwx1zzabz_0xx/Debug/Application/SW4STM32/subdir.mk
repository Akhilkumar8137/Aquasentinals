################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
/home/icfoss/Desktop/STM/newDo_ph/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/startup_stm32l072xx.s 

C_SRCS += \
/home/icfoss/Desktop/STM/newDo_ph/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/syscalls.c 

OBJS += \
./Application/SW4STM32/startup_stm32l072xx.o \
./Application/SW4STM32/syscalls.o 

S_DEPS += \
./Application/SW4STM32/startup_stm32l072xx.d 

C_DEPS += \
./Application/SW4STM32/syscalls.d 


# Each subdirectory must supply rules for building sources it contributes
Application/SW4STM32/startup_stm32l072xx.o: /home/icfoss/Desktop/STM/newDo_ph/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/startup_stm32l072xx.s Application/SW4STM32/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m0plus -g3 -c -I"/home/icfoss/Desktop/STM/newDo_ph/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/cmwx1zzabz_0xx/Application/do_sensor" -I"/home/icfoss/Desktop/STM/newDo_ph/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/cmwx1zzabz_0xx/Application/pH_sensor" -x assembler-with-cpp -MMD -MP -MF"Application/SW4STM32/startup_stm32l072xx.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"
Application/SW4STM32/syscalls.o: /home/icfoss/Desktop/STM/newDo_ph/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/syscalls.c Application/SW4STM32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32L072xx -DUSE_B_L072Z_LRWAN1 -DCMWX1ZZABZ0XX -c -I../../../LoRaWAN/App -I../../../LoRaWAN/Target -I../../../Core/Inc -I../../../Utilities/misc -I../../../Utilities/timer -I../../../Utilities/trace/adv_trace -I../../../Utilities/lpm/tiny_lpm -I../../../Utilities/sequencer -I../../../Drivers/BSP/B-L072Z-LRWAN1 -I../../../Drivers/BSP/CMWX1ZZABZ_0xx -I../../../Drivers/STM32L0xx_HAL_Driver/Inc -I../../../Drivers/CMSIS/Device/ST/STM32L0xx/Include -I../../../Drivers/CMSIS/Include -I../../../Middlewares/Third_Party/SubGHz_Phy -I../../../Middlewares/Third_Party/SubGHz_Phy/sx1276 -I../../../Middlewares/Third_Party/LoRaWAN/Crypto -I../../../Middlewares/Third_Party/LoRaWAN/Mac -I../../../Middlewares/Third_Party/LoRaWAN/Mac/Region -I../../../Middlewares/Third_Party/LoRaWAN/Utilities -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler -I../../../Middlewares/Third_Party/LoRaWAN/LmHandler/Packages -I"/home/icfoss/Desktop/STM/newDo_ph/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/cmwx1zzabz_0xx/Application/do_sensor" -I"/home/icfoss/Desktop/STM/newDo_ph/dissolved_oxygen_sensor/c-1_dev_lorawan_end_node_firmware_v2.1-main(3)/c-1_dev_lorawan_end_node_firmware_v2.1-main/Software/LoRaWAN_End_Node/STM32CubeIDE/cmwx1zzabz_0xx/Application/pH_sensor" -Os -ffunction-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"Application/SW4STM32/syscalls.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Application-2f-SW4STM32

clean-Application-2f-SW4STM32:
	-$(RM) ./Application/SW4STM32/startup_stm32l072xx.d ./Application/SW4STM32/startup_stm32l072xx.o ./Application/SW4STM32/syscalls.cyclo ./Application/SW4STM32/syscalls.d ./Application/SW4STM32/syscalls.o ./Application/SW4STM32/syscalls.su

.PHONY: clean-Application-2f-SW4STM32

