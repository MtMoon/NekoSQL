################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../sysmanager/SysManager.cpp 

OBJS += \
./sysmanager/SysManager.o 

CPP_DEPS += \
./sysmanager/SysManager.d 


# Each subdirectory must supply rules for building sources it contributes
sysmanager/%.o: ../sysmanager/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


