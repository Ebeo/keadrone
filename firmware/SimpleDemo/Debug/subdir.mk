################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cr_startup_lpc17.c \
../main.c \
../uart.c 

OBJS += \
./cr_startup_lpc17.o \
./main.o \
./uart.o 

C_DEPS += \
./cr_startup_lpc17.d \
./main.d \
./uart.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC17xx -D__CODE_RED -D__REDLIB__ -I"C:\Users\admin\Documents\lpcxpresso_3.6\workspace\SimpleDemo" -I"C:\Users\admin\Documents\lpcxpresso_3.6\workspace\CMSISv1p30_LPC17xx\inc" -I"C:\Users\admin\Documents\lpcxpresso_3.6\workspace\FreeRTOS_Library\include" -I"C:\Users\admin\Documents\lpcxpresso_3.6\workspace\FreeRTOS_Library\portable" -O0 -g3 -fsigned-char -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

