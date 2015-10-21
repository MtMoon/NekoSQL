################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Tool/RecordTool.cpp 

OBJS += \
./Tool/RecordTool.o 

CPP_DEPS += \
./Tool/RecordTool.d 


# Each subdirectory must supply rules for building sources it contributes
Tool/%.o: ../Tool/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


