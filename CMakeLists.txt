#本文件由自动化脚本生成,改这个文件是没用的,要更改配置请修改项目根目录下的 project_config.json 文件,然后再次运行脚本,即可更改编译配置
#未经作者允许,不可更改脚本内容,不可传播,脚本源码闭源,只可在实验室内部使用,否则本人线下追究责任,脚本版权归作者本人和实验室所有
cmake_minimum_required(VERSION 3.16)

# 项目配置
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m3)
set(TARGET_CHIP STM32F103C8)
set(TARGET_BOARD BLUEPILL)

# 工具链
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(Temp32Project C CXX ASM)

# 编译标志 (软件浮点)
set(CMAKE_C_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -fdata-sections -ffunction-sections -Wall -fstack-usage -O0 -g3")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -fdata-sections -ffunction-sections -Wall -fstack-usage -O0 -g3")
set(CMAKE_ASM_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -fdata-sections -ffunction-sections -Wall -fstack-usage -O0 -g3")

# 链接标志
set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -specs=nosys.specs -specs=nano.specs -T${CMAKE_SOURCE_DIR}/BSP/CORE/stm32f10x.ld -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map,--cref -u _printf_float -Wl,--print-memory-usage")

# 宏定义
add_definitions(-DSTM32F10X_MD)
add_definitions(-DSTM32F103C8__Chipinfo__)

# 头文件目录
include_directories(
    Control
    Driver
    Driver_Framework
    app
    BSP/CORE
    Driver/flash
    Driver_Framework/i2c
    Driver_Framework/key
    Driver_Framework/led
    Middleware/shell
    Middleware/trans
    BSP/CMSIS/Core/Include
    BSP/CMSIS/Core/Include/a-profile
    BSP/CMSIS/Core/Include/m-profile
    BSP/CMSIS/Core/Include/r-profile
)

# 源文件分组
set(CORE_SOURCES
    BSP/CORE/startup_stm32f10x_md.c
    BSP/CORE/stm32f10x_it.c
    BSP/CORE/syscalls.c
    BSP/CORE/sysmem.c
    BSP/CORE/system_stm32f10x.c
)

set(CONTROL_SOURCES
    Control/filter.c
    Control/pid.c
)

set(DRIVER_SOURCES
    Driver/Systick.c
    Driver/adc.c
    Driver/delay.c
    Driver/led.c
    Driver/misc.c
    Driver/nvic.c
    Driver/usart.c
)

set(DRIVER_FRAMEWORK_SOURCES
    Driver_Framework/dev_model_core.c
)

set(APP_SOURCES
    app/control.c
    app/init.c
    app/irq.c
    app/main.c
)

set(FLASH_SOURCES
    Driver/flash/flash.c
    Driver/flash/flash_example.c
    Driver/flash/flash_main_test.c
    Driver/flash/flash_space_test.c
    Driver/flash/flash_test.c
)

set(LED_SOURCES
    Driver_Framework/led/df_led.c
)

# 所有源文件
set(SOURCE_FILES
    ${CORE_SOURCES}
    ${CONTROL_SOURCES}
    ${DRIVER_SOURCES}
    ${DRIVER_FRAMEWORK_SOURCES}
    ${APP_SOURCES}
    ${FLASH_SOURCES}
    ${LED_SOURCES}
)

# 子目录
add_subdirectory(Driver_Framework/i2c)
add_subdirectory(Driver_Framework/key)
add_subdirectory(Middleware/shell)
add_subdirectory(Middleware/trans)

# 创建可执行文件
add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

# 链接静态库
target_link_libraries(${PROJECT_NAME}.elf
    i2c
    key
    shell
    trans
)

# 生成输出文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Building output files"
)
