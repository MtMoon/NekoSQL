################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../DataManagerTest.cpp \
../IndexManagerTest.cpp \
../IndexManagerTest2.cpp \
../SynAnalyzeTest.cpp \
../SysManagerTest.cpp 

OBJS += \
./DataManagerTest.o \
./IndexManagerTest.o \
./IndexManagerTest2.o \
./SynAnalyzeTest.o \
./SysManagerTest.o 

CPP_DEPS += \
./DataManagerTest.d \
./IndexManagerTest.d \
./IndexManagerTest2.d \
./SynAnalyzeTest.d \
./SysManagerTest.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


