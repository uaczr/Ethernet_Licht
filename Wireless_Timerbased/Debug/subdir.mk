################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BeatByNiko.cpp \
../BeatRec.cpp \
../BeatStat.cpp \
../BehaviourGenerator.cpp \
../Communicator.cpp \
../Equipment.cpp \
../Musikanalysis.cpp \
../Spectrum.cpp \
../main.cpp 

OBJS += \
./BeatByNiko.o \
./BeatRec.o \
./BeatStat.o \
./BehaviourGenerator.o \
./Communicator.o \
./Equipment.o \
./Musikanalysis.o \
./Spectrum.o \
./main.o 

CPP_DEPS += \
./BeatByNiko.d \
./BeatRec.d \
./BeatStat.d \
./BehaviourGenerator.d \
./Communicator.d \
./Equipment.d \
./Musikanalysis.d \
./Spectrum.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1y -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


