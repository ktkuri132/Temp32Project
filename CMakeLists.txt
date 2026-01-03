#本文件由自动化脚本生成,改这个文件是没用的,要更改配置请修改项目根目录下的 project_config.json 文件,然后再次运行脚本,即可更改编译配置
#未经作者允许,不可更改脚本内容,不可传播,脚本源码闭源,只可在实验室内部使用,否则本人线下追究责任,脚本版权归作者本人和实验室所有
cmake_minimum_required(VERSION 3.16)

# 项目配置
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m3)
set(TARGET_CHIP STM32F103C8T6)
set(TARGET_BOARD BLUEPILL)
set(CHIP_PACKAGE "c8t6")  # 芯片封装型号 (如vet6, vgt6等)

# BSP芯片目录
set(BSP_CHIP_DIR "stm32f1")
set(BSP_MODEL_DIR "f103")  # 芯片型号目录
set(BSP_PACKAGE_DIR "c8t6")  # 封装特定驱动目录

# 工具链
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(General_template_Project C CXX ASM)

# 编译标志 (软件浮点)
set(CMAKE_C_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -fdata-sections -ffunction-sections -fstack-usage -O0 -g3")
set(CMAKE_CXX_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -fdata-sections -ffunction-sections -fstack-usage -O0 -g3")
set(CMAKE_ASM_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -fdata-sections -ffunction-sections -fstack-usage -O0 -g3")

# 链接标志
set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m3 -mthumb -mfloat-abi=soft -specs=nosys.specs -specs=nano.specs -T${CMAKE_SOURCE_DIR}/BSP/stm32f1/f103/c8t6/stm32f1c8t6.ld -T${CMAKE_SOURCE_DIR}/Driver_Framework/linker/df_init_sections.ld -Wl,--gc-sections -Wl,-Map=${PROJECT_NAME}.map,--cref -u _printf_float -u _scanf_float -Wl,--print-memory-usage")

# 宏定义
add_definitions(-DLOG_USE_COLOR)
add_definitions(-DSTM32F40_41xxx)
add_definitions(-DSTM32F10X_MD)
add_definitions(-DCHIP_MODEL_F103)     # 芯片型号: f103
add_definitions(-DCHIP_PACKAGE_C8T6)   # 芯片封装型号: c8t6
add_definitions(-DCHIP_PACKAGE="c8t6") # 芯片封装字符串
add_definitions(-DUSE_DEVICE__ROOT)  # 启用_root设备驱动
add_definitions(-DUSE_DEVICE_BMP280)  # 启用bmp280设备驱动
add_definitions(-DUSE_DEVICE_HMC588)  # 启用hmc588设备驱动
add_definitions(-DUSE_DEVICE_MPU6050)  # 启用mpu6050设备驱动
add_definitions(-DUSE_DEVICE_SH1106)  # 启用sh1106设备驱动
add_definitions(-DUSE_DEVICE_SSD1306)  # 启用ssd1306设备驱动
add_definitions(-DUSE_DEVICE_ST7789)  # 启用st7789设备驱动

# BSP层(板级支持包)
add_subdirectory(BSP)

# Device层(外部设备驱动)
add_subdirectory(Device)

# 应用层和框架层头文件目录
include_directories(
    app
    Control
    Driver_Framework
    Driver_Framework/display
    Driver_Framework/i2c
    Driver_Framework/irq
    Driver_Framework/key
    Driver_Framework/lcd
    Driver_Framework/shell
    Driver_Framework/spi
    Middleware
    Middleware/trans
)
# BSP和Device层头文件目录由子目录CMakeLists.txt导出
include_directories(Device)  # 添加Device根目录用于<config.h>
include_directories(${BSP_INCLUDE_DIRS})
include_directories(${DEVICE_INCLUDE_DIRS})

set(CONTROL_SOURCES
    Control/filter.c
    Control/pid.c
)

set(DRIVER_FRAMEWORK_SOURCES
    Driver_Framework/dev_frame.c
    Driver_Framework/df_init.c
    Driver_Framework/df_log.c
)

set(DRIVER_FRAMEWORK_DISPLAY_SOURCES
    Driver_Framework/display/df_display.c
)

set(DRIVER_FRAMEWORK_I2C_SOURCES
    Driver_Framework/i2c/df_iic.c
)

set(DRIVER_FRAMEWORK_IRQ_SOURCES
    Driver_Framework/irq/df_irq.c
)

set(DRIVER_FRAMEWORK_KEY_SOURCES
    Driver_Framework/key/df_key.c
)

set(DRIVER_FRAMEWORK_LCD_SOURCES
    Driver_Framework/lcd/df_fonts.c
    Driver_Framework/lcd/df_lcd.c
)

set(DRIVER_FRAMEWORK_SHELL_SOURCES
    Driver_Framework/shell/df_shell.c
)

set(DRIVER_FRAMEWORK_SPI_SOURCES
    Driver_Framework/spi/df_spi.c
)

set(MIDDLEWARE_TRANS_SOURCES
    Middleware/trans/stde.c
    Middleware/trans/terminal_link.c
)

set(APP_SOURCES
    app/control.c
    app/init.c
    app/main.c
    app/test_auto_init.c
)

# 所有源文件
set(SOURCE_FILES
    ${CONTROL_SOURCES}
    ${DRIVER_FRAMEWORK_SOURCES}
    ${DRIVER_FRAMEWORK_DISPLAY_SOURCES}
    ${DRIVER_FRAMEWORK_I2C_SOURCES}
    ${DRIVER_FRAMEWORK_IRQ_SOURCES}
    ${DRIVER_FRAMEWORK_KEY_SOURCES}
    ${DRIVER_FRAMEWORK_LCD_SOURCES}
    ${DRIVER_FRAMEWORK_SHELL_SOURCES}
    ${DRIVER_FRAMEWORK_SPI_SOURCES}
    ${MIDDLEWARE_TRANS_SOURCES}
    ${APP_SOURCES}
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