################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CU_SRCS += \
../src/apps/snap/snap-global.cu \
../src/apps/snap/snap-kernel.cu \
../src/apps/snap/snap-run.cu \
../src/apps/snap/snap-task.cu 

CU_DEPS += \
./src/apps/snap/snap-global.d \
./src/apps/snap/snap-kernel.d \
./src/apps/snap/snap-run.d \
./src/apps/snap/snap-task.d 

OBJS += \
./src/apps/snap/snap-global.o \
./src/apps/snap/snap-kernel.o \
./src/apps/snap/snap-run.o \
./src/apps/snap/snap-task.o 


# Each subdirectory must supply rules for building sources it contributes
src/apps/snap/%.o: ../src/apps/snap/%.cu
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/opt/cuda/8.0/cuda/bin/nvcc -I/usr/local/cuda/include -I/usr/local/cuda/samples/common/inc/ -G -g -lineinfo -O0 -std=c++11 -gencode arch=compute_60,code=sm_60 -m64 -odir "src/apps/snap" -M -o "$(@:%.o=%.d)" "$<"
	/opt/cuda/8.0/cuda/bin/nvcc -I/usr/local/cuda/include -I/usr/local/cuda/samples/common/inc/ -G -g -lineinfo -O0 -std=c++11 --compile --relocatable-device-code=true -gencode arch=compute_60,code=compute_60 -gencode arch=compute_60,code=sm_60 -m64  -x cu -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


