#include "main.h"
#include <stdint.h>

// 声明flash测试函数
void flash_info_query(void);
uint32_t a = 0;
int main() {

    // dev_model_core_init(Drivers_model_poor);        // 初始化设备模型
    // MCU_Shell_Init(&Shell, &STM32F103C8T6_Device);  // 初始化Shell

    // 运行flash测试
    // flash_info_query();

    while (1) {
        // Task_Switch_Tick_Handler(&Shell_Sysfpoint);
        a++;
    }
    return 0;
}
