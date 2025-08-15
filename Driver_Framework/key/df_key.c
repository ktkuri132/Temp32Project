#include "df_key.h"

/**
 * @brief 按键状态读取（阻塞）
 *
 * @param func 执行的函数
 */
bool Key_ReadBlocking(Kert *key_evet_handler, Kt *key, void (*func)(void)) {
    // 持续读取按键状态，直到按键被释放
    while (key_evet_handler->Key_Read(key)) {
    }
    if (func == NULL) {
        return true;  // 如果没有传入函数，则直接返回
    }
    func();  // 调用传入的函数
    return true;
}

/**
 * @brief 按键定时器函数
 * @param key_evet_handler 按键事件句柄
 */
void Key_Tick(Kert *key_evet_handler) {
    key_evet_handler->Sys_RunTime++;  // 增加系统运行时间
    // 这里可以添加其他定时任务或处理逻辑
    // 例如，检查按键状态，处理按键事件等
}

/**
 * @brief 按键中断处理函数
 *
 */
void Key_IRQHandler(Kert *key_evet_handler, Kt *key) {
    if (key_evet_handler->Key_Read(key)) {
        key->Key_Pressed = true;
        key->Key_Press_Count++;
        key->Key_Pressed_Time = key_evet_handler->Sys_RunTime;
    } else {
        if (key->Key_Pressed) {
            key->Key_Pressed = false;
            key->Key_Pressed_Time = key_evet_handler->Sys_RunTime -
                                    key->Key_Pressed_Time;  // 计算按下时间
            key->Key_ok =
                true;  // 标记一次按键动作结束，为true则表示一次动作结束挂起
        }
    }
}

/**
 * @brief 按键事件处理函数
 *
 * @param key 按键结构体指针
 * @param TimeOut 超时时间
 * @param current_time 当前时间
 * @param Default_Pressed_CallBack 默认按下回调函数
 * @param TimeOut_Pressed_CallBack 超时按下回调函数
 * @param Default_Release_CallBack 默认释放回调函数
 * @param TimeOut_Release_CallBack 超时释放回调函数
 * @return uint8_t
 */
uint8_t Key_Event(Kert *key_evet_handler, Kt *key, uint32_t TimeOut,
                  void (*Default_Pressed_CallBack)(void),
                  void (*TimeOut_Pressed_CallBack)(void),
                  void (*Default_Release_CallBack)(void),
                  void (*TimeOut_Release_CallBack)(void)) {
    static bool Default_Pressed_CallBack_Run_Flag = false;
    if (key->Key_Pressed) {
        if (!Default_Pressed_CallBack_Run_Flag) {  // 检查默认按下回调
            if (Default_Pressed_CallBack !=
                NULL) {  // 如果有默认按下回调函数，则调用
                Default_Pressed_CallBack();
                Default_Pressed_CallBack_Run_Flag =
                    true;  // 设置标志位，表示默认按下回调函数已运行
            }
        }
        if (TimeOut) {  // 检查超时回调，初步根据TimeOut参数判断是否存在超时回调
            if ((key_evet_handler->Sys_RunTime - key->Key_Pressed_Time) >
                TimeOut) {  // 检查超时回调
                if (TimeOut_Pressed_CallBack !=
                    NULL) {  // 如果有按键超时回调函数，则调用,避免出现空指针异常
                    key->Key_LS_State = true;  // 设置为长按状态
                    TimeOut_Pressed_CallBack();
                }
                return 0;  // 如果按键被按下超过1秒，返回0
            }
        }
    } else {
        if (key->Key_ok) {
            if (Default_Release_CallBack !=
                NULL) {  // 如果有按键释放回调函数，则调用
                Default_Release_CallBack();
            }
            if (key->Key_LS_State) {
                if (TimeOut_Release_CallBack != NULL) {
                    TimeOut_Release_CallBack();
                }
            }
            key->Key_ok = false;        // 清除按键状态
            key->Key_Press_Count = 0;   // 清除按键计数
            key->Key_Pressed_Time = 0;  // 清除按键按下时间
            // printf("KEY_%d Pressed\n", key_num);
            return 1;  // 如果按键被释放，返回1
        }
    }
    return 2;  // 默认返回2
}