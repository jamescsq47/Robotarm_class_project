cmake_minimum_required(VERSION 3.22)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/gcc-arm-none-eabi.cmake" CACHE FILEPATH "")
project(robotarm C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
file(GLOB HAL_SOURCES CONFIGURE_DEPENDS Drivers/STM32F4xx_HAL_Driver/Src/*.c)
add_executable(robotarm Core/Src/main.c Core/Src/runtime.c Core/Src/tim.c Core/Src/servo.c Core/Src/servo_motion.c Core/Src/robotarm_actions.c Core/Src/system_stm32f4xx.c startup_stm32f407xx.s ${HAL_SOURCES})
set_target_properties(robotarm PROPERTIES SUFFIX ".elf")
target_include_directories(robotarm PRIVATE Core/Inc Drivers/STM32F4xx_HAL_Driver/Inc Drivers/CMSIS/Include Drivers/CMSIS/Device/ST/STM32F4xx/Include)
target_compile_definitions(robotarm PRIVATE USE_HAL_DRIVER STM32F407xx)
target_compile_options(robotarm PRIVATE -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Wall -Wextra -ffunction-sections -fdata-sections -Og -g3)
target_link_options(robotarm PRIVATE -mcpu=cortex-m4 -mthumb -mfloat-abi=soft "-T${CMAKE_SOURCE_DIR}/STM32F407ZGTX_FLASH.ld" -nostdlib -Wl,--build-id=none -Wl,--gc-sections -Wl,-Map=robotarm.map)
add_custom_command(TARGET robotarm POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:robotarm> ${CMAKE_CURRENT_BINARY_DIR}/robotarm.hex COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:robotarm> ${CMAKE_CURRENT_BINARY_DIR}/robotarm.bin COMMAND ${CMAKE_SIZE} $<TARGET_FILE:robotarm>)

target_link_libraries(robotarm PRIVATE gcc)



