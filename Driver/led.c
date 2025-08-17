#include "driver.h"
void LED_Init(void) {
    // 1. 使能GPIOC时钟
    RCC->APB2ENR |= (1 << 4);  // IOPCEN = 1

    // 2. 配置PC13为推挽输出50MHz
    // CRH控制PC8~PC15，每4位控制一个引脚
    // 先清零PC13对应的4位
    GPIOC->CRH &= ~(0xF << 20);  // 20~23位
    // 设置MODE13[1:0]=11(50MHz输出)，CNF13[1:0]=00(推挽输出)
    GPIOC->CRH |= (0x3 << 20);

    // 3. 初始化PC13为低电平
    GPIOC->ODR &= ~(1 << 13);
}

void LED_Toggle(void) {
    // 切换PC11的状态
    GPIOC->ODR ^= (1 << 13);
}

void LED_Off(void) {
    // 设置PC11为高电平
    GPIOC->ODR |= (1 << 13);
}

void LED_On(void) {
    // 设置PC11为低电平
    GPIOC->ODR &= ~(1 << 13);
}

int led_init(dev_arg_t arg) {
    (void)arg;  // 忽略参数
    LED_Init();  // 调用LED初始化函数
    return 0;    // 返回0表示成功
}

int led_enable(dev_arg_t arg) {
    (void)arg;  // 忽略参数
    LED_On();
    return 0;  // 返回0表示成功
}

int led_disable(dev_arg_t arg) {
    (void)arg;  // 忽略参数
    LED_Off();
    return 0;  // 返回0表示成功
}