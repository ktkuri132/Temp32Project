#include "main.h"
#include <stdint.h>
#include <stdio.h>

// 用于实时监视演示的变量
volatile uint32_t counter = 0;           // 计数器 - 添加到监视窗口
volatile float temperature = 25.0f;      // 模拟温度值 - 实时监视
volatile int system_state = 0;           // 系统状态 - 监视状态变化
volatile uint16_t adc_value = 512;       // ADC 采样值 - 波形显示

// SWO 输出重定向 (用于 printf 调试)
int _write(int file, char *ptr, int len) {
    // 检查 ITM 是否可用
    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) &&
        (ITM->TCR & ITM_TCR_ITMENA_Msk) &&
        (ITM->TER & 1UL)) {
        
        for (int i = 0; i < len; i++) {
            while (ITM->PORT[0].u32 == 0UL) {
                __NOP();
            }
            ITM->PORT[0].u8 = (uint8_t)*ptr++;
        }
    }
    return len;
}

// 初始化 SWO 调试输出
void SWO_Init(void) {
    // 启用 Trace 功能
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    
    // 解锁 ITM
    ITM->LAR = 0xC5ACCE55;
    
    // 启用 ITM 端口 0
    ITM->TER = 0x00000001;
    
    // 配置 ITM 控制寄存器
    ITM->TCR = (1 << ITM_TCR_TraceBusID_Pos) |  // Trace Bus ID
               (1 << ITM_TCR_DWTENA_Pos) |      // DWT 启用
               (1 << ITM_TCR_SYNCENA_Pos) |     // 同步包启用
               (1 << ITM_TCR_ITMENA_Pos);       // ITM 启用
}

// 模拟传感器读取
void simulate_sensor_data(void) {
    // 模拟温度传感器 (正弦波 + 噪声)
    static float time = 0;
    time += 0.1f;
    temperature = 25.0f + 10.0f * sin(time) + (rand() % 20 - 10) * 0.1f;
    
    // 模拟 ADC 采样 (锯齿波)
    static int direction = 1;
    adc_value += direction * 10;
    if (adc_value >= 4000) direction = -1;
    if (adc_value <= 100) direction = 1;
    
    // 系统状态循环
    if (counter % 1000 == 0) {
        system_state = (system_state + 1) % 4;  // 0-3 循环
    }
}

// 声明flash测试函数
void flash_info_query(void);

int main() {
    // 初始化 SWO 调试输出
    SWO_Init();
    
    // 发送启动消息
    printf("=== STM32F103C8 实时监视演示 ===\n");
    printf("SWO 调试输出已启用\n");
    printf("请在 VS Code 中监视以下变量:\n");
    printf("- counter: 主循环计数器\n");
    printf("- temperature: 模拟温度值\n");
    printf("- system_state: 系统状态\n");
    printf("- adc_value: ADC 采样值\n");
    printf("=============================\n\n");

    // 主循环
    while (1) {
        // 增加计数器
        counter++;
        
        // 模拟传感器数据
        simulate_sensor_data();
        
        // 每1000次循环输出一次状态信息
        if (counter % 1000 == 0) {
            printf("循环: %lu, 温度: %.1f°C, 状态: %d, ADC: %d\n", 
                   counter, temperature, system_state, adc_value);
        }
        
        // 简单延时 (在实际项目中应该使用定时器)
        for (volatile int i = 0; i < 10000; i++) {
            __NOP();
        }
    }
    
    return 0;
}

/*
使用说明：

1. 编译并下载这个程序到 STM32F103C8

2. 在 VS Code 中启动调试 (F5)

3. 在 Watch 面板中添加以下变量进行实时监视：
   - counter
   - temperature
   - system_state
   - adc_value

4. 在调试控制台的 "ITM" 标签页中查看 printf 输出

5. 观察变量值的实时变化：
   - counter: 持续递增
   - temperature: 正弦波变化 (25±10°C)
   - system_state: 每1000次循环切换 (0-3)
   - adc_value: 锯齿波变化 (100-4000)

6. 实时监视的优势：
   - 程序持续运行，不需要暂停
   - 变量值实时更新显示
   - 可以观察变化趋势和模式
   - SWO 输出不影响程序时序

注意：
- 确保 J-Link 的 SWO 引脚正确连接
- 如果看不到 SWO 输出，检查硬件连接
- 变量使用 volatile 修饰避免编译器优化
*/
