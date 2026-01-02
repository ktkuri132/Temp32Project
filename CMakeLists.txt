#本文件由自动化脚本生成,改这个文件是没用的,要更改配置请修改项目根目录下的 project_config.json 文件,然后再次运行脚本,即可更改编译配置
#未经作者允许,不可更改脚本内容,不可传播,脚本源码闭源,只可在实验室内部使用,否则本人线下追究责任,脚本版权归作者本人和实验室所有
cmake_minimum_required(VERSION 3.16)

# 项目配置
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m4)
set(TARGET_CHIP STM32F4)
set(TARGET_BOARD BLUEPILL)

# BSP芯片目录
set(BSP_CHIP_DIR "stm32f4")

# 工具链
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(General_template_Project C CXX ASM)

# 编译标志 (硬件浮点 (FPU: fpv4-sp-d16))
set(CMAKE_C_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fdata-sections -ffunction-sections -fstack-usage -O0 -g3")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fdata-sections -ffunction-sections -fstack-usage -O0 -g3")
set(CMAKE_ASM_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fdata-sections -ffunction-sections -fstack-usage -O0 -g3")

# 链接标志
set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -specs=nosys.specs -specs=nano.specs -T${CMAKE_SOURCE_DIR}/BSP/stm32f4/CORE/stm32f4xx.ld -T${CMAKE_SOURCE_DIR}/Driver_Framework/linker/df_init_sections.ld -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map,--cref -u _printf_float -u _scanf_float -Wl,--print-memory-usage")

# 宏定义
add_definitions(-DSTM32F40_41xxx)
add_definitions(-DSTM32F10X_MD)
add_definitions(-DLOG_USE_COLOR)
add_definitions(-DUSE_DEVICE_BMP280)  # 启用bmp280设备驱动
add_definitions(-DUSE_DEVICE_HMC588)  # 启用hmc588设备驱动
add_definitions(-DUSE_DEVICE_MPU6050)  # 启用mpu6050设备驱动
add_definitions(-DUSE_DEVICE_SH1106)  # 启用sh1106设备驱动

# BSP层(板级支持包)
add_subdirectory(BSP)

# Device层(外部设备驱动)
add_subdirectory(Device)

# 应用层和框架层头文件目录
include_directories(
    Control
    Driver_Framework
    Middleware
    app
    Driver_Framework/display
    Driver_Framework/i2c
    Driver_Framework/irq
    Driver_Framework/key
    Driver_Framework/lcd
    Driver_Framework/spi
    Middleware/shell
    Middleware/trans
)
# BSP和Device层头文件目录由子目录CMakeLists.txt导出
include_directories(Device)  # 添加Device根目录用于<config.h>
include_directories(${BSP_INCLUDE_DIRS})
include_directories(${DEVICE_INCLUDE_DIRS})

# 源文件分组
set(CONTROL_SOURCES
    Control/filter.c
    Control/pid.c
)

set(DRIVER_FRAMEWORK_SOURCES
    Driver_Framework/dev_frame.c
    Driver_Framework/df_init.c
    Driver_Framework/df_log.c
)

set(APP_SOURCES
    app/control.c
    app/init.c
    app/main.c
    app/test_auto_init.c
)

set(DISPLAY_SOURCES
    Driver_Framework/display/df_display.c
)

set(I2C_SOURCES
    Driver_Framework/i2c/df_iic.c
)

set(IRQ_SOURCES
    Driver_Framework/irq/df_irq.c
)

set(KEY_SOURCES
    Driver_Framework/key/df_key.c
)

set(LCD_SOURCES
    Driver_Framework/lcd/df_fonts.c
    Driver_Framework/lcd/df_lcd.c
)

set(SHELL_SOURCES
    Middleware/shell/shell.c
)

set(SPI_SOURCES
    Driver_Framework/spi/df_spi.c
)

set(TRANS_SOURCES
    Middleware/trans/stde.c
    Middleware/trans/terminal_link.c
)

# 所有源文件
set(SOURCE_FILES
    ${CONTROL_SOURCES}
    ${DRIVER_FRAMEWORK_SOURCES}
    ${APP_SOURCES}
    ${DISPLAY_SOURCES}
    ${I2C_SOURCES}
    ${IRQ_SOURCES}
    ${KEY_SOURCES}
    ${LCD_SOURCES}
    ${SHELL_SOURCES}
    ${SPI_SOURCES}
    ${TRANS_SOURCES}
)

# 创建可执行文件
add_executable(${PROJECT_NAME}.elf
    ${SOURCE_FILES}
    ${BSP_SOURCES}      # BSP层源文件
    ${DEVICE_SOURCES}   # Device层源文件
)

# 生成输出文件
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${PROJECT_NAME}.bin
    COMMAND ${SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Building output files"
)
