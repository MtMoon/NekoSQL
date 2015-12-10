################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../indexmanager/BPlusTree.cpp \
../indexmanager/BTNode.cpp \
../indexmanager/IndexManager.cpp 

OBJS += \
./indexmanager/BPlusTree.o \
./indexmanager/BTNode.o \
./indexmanager/IndexManager.o 

CPP_DEPS += \
./indexmanager/BPlusTree.d \
./indexmanager/BTNode.d \
./indexmanager/IndexManager.d 


# Each subdirectory must supply rules for building sources it contributes
indexmanager/%.o: ../indexmanager/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


