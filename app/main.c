#include "main.h"

int main() {
    dev_model_core_init(Drivers_model_poor);        // 初始化设备模型
    MCU_Shell_Init(&Shell, &STM32F103C8T6_Device);  // 初始化Shell
    while (1) {
        Task_Switch_Tick_Handler(&Shell_Sysfpoint);
    }
    return 0;
}
