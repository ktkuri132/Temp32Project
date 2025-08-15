#include "df_led.h"

void LED_Blink(Lt *led, Lert *led_event_handler, uint32_t blink_time) {
    led->LED_Blink_Time = blink_time;  // 设置闪烁时间间隔
    led_event_handler->Sys_RunTime = 0;  // 重置系统运行时间

    // 初始化LED
    if (!led->LED_Init_Flag) {
        led->Init();
    }

    // 开始闪烁
    while (led_event_handler->Sys_RunTime < blink_time) {
        if (led->LED_State) {
            if (led->Off) {
                led->Off();  // 关闭LED
            }
            led->LED_State = false;
        } else {
            if (led->On) {
                led->On();  // 打开LED
            }
            led->LED_State = true;
        }

        // 模拟延时（实际应用中应使用定时器或其他方式）
        for (volatile uint32_t i = 0; i < 100000; i++) {
            ;  // 简单的忙等待延时
        }

        led_event_handler->Sys_RunTime += led->LED_Blink_Time;  // 更新系统运行时间
    }
}