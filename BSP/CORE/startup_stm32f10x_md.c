#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;

void Reset_Handler(void);
void Default_Handler(void);
void main(void);
void SystemInit(void);
void __libc_init_array(void);

// 弱定义所有中断向量
void NMI_Handler(void)                __attribute__ ((weak, alias ("Default_Handler")));
void HardFault_Handler(void)          __attribute__ ((weak, alias ("Default_Handler")));
void MemManage_Handler(void)          __attribute__ ((weak, alias ("Default_Handler")));
void BusFault_Handler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void UsageFault_Handler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void SVC_Handler(void)                __attribute__ ((weak, alias ("Default_Handler")));
void DebugMon_Handler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void PendSV_Handler(void)             __attribute__ ((weak, alias ("Default_Handler")));
void SysTick_Handler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void WWDG_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void PVD_IRQHandler(void)             __attribute__ ((weak, alias ("Default_Handler")));
void TAMPER_IRQHandler(void)          __attribute__ ((weak, alias ("Default_Handler")));
void RTC_IRQHandler(void)             __attribute__ ((weak, alias ("Default_Handler")));
void FLASH_IRQHandler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void RCC_IRQHandler(void)             __attribute__ ((weak, alias ("Default_Handler")));
void EXTI0_IRQHandler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void EXTI1_IRQHandler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void EXTI2_IRQHandler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void EXTI3_IRQHandler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void EXTI4_IRQHandler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Channel1_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Channel2_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Channel3_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Channel4_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Channel5_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Channel6_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void DMA1_Channel7_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void ADC1_2_IRQHandler(void)          __attribute__ ((weak, alias ("Default_Handler")));
void USB_HP_CAN1_TX_IRQHandler(void)  __attribute__ ((weak, alias ("Default_Handler")));
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__ ((weak, alias ("Default_Handler")));
void CAN1_RX1_IRQHandler(void)        __attribute__ ((weak, alias ("Default_Handler")));
void CAN1_SCE_IRQHandler(void)        __attribute__ ((weak, alias ("Default_Handler")));
void EXTI9_5_IRQHandler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void TIM1_BRK_IRQHandler(void)        __attribute__ ((weak, alias ("Default_Handler")));
void TIM1_UP_IRQHandler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void TIM1_TRG_COM_IRQHandler(void)    __attribute__ ((weak, alias ("Default_Handler")));
void TIM1_CC_IRQHandler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void TIM2_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void TIM3_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void TIM4_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void I2C1_EV_IRQHandler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void I2C1_ER_IRQHandler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void I2C2_EV_IRQHandler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void I2C2_ER_IRQHandler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void SPI1_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void SPI2_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void USART1_IRQHandler(void)          __attribute__ ((weak, alias ("Default_Handler")));
void USART2_IRQHandler(void)          __attribute__ ((weak, alias ("Default_Handler")));
void USART3_IRQHandler(void)          __attribute__ ((weak, alias ("Default_Handler")));
void EXTI15_10_IRQHandler(void)       __attribute__ ((weak, alias ("Default_Handler")));
void RTCAlarm_IRQHandler(void)        __attribute__ ((weak, alias ("Default_Handler")));
void USBWakeUp_IRQHandler(void)       __attribute__ ((weak, alias ("Default_Handler")));
// F105/F107新增中断
void TIM8_BRK_IRQHandler(void)        __attribute__ ((weak, alias ("Default_Handler")));
void TIM8_UP_IRQHandler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void TIM8_TRG_COM_IRQHandler(void)    __attribute__ ((weak, alias ("Default_Handler")));
void TIM8_CC_IRQHandler(void)         __attribute__ ((weak, alias ("Default_Handler")));
void ADC3_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void FSMC_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void SDIO_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void TIM5_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void SPI3_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void UART4_IRQHandler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void UART5_IRQHandler(void)           __attribute__ ((weak, alias ("Default_Handler")));
void TIM6_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void TIM7_IRQHandler(void)            __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Channel1_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Channel2_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Channel3_IRQHandler(void)   __attribute__ ((weak, alias ("Default_Handler")));
void DMA2_Channel4_5_IRQHandler(void) __attribute__ ((weak, alias ("Default_Handler")));

// 向量表
__attribute__ ((section(".isr_vector"), used))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),           // 0  Top of Stack
    Reset_Handler,                        // 1  Reset Handler
    NMI_Handler,                          // 2  NMI Handler
    HardFault_Handler,                    // 3  Hard Fault Handler
    MemManage_Handler,                    // 4  MPU Fault Handler
    BusFault_Handler,                     // 5  Bus Fault Handler
    UsageFault_Handler,                   // 6  Usage Fault Handler
    0, 0, 0, 0,                           // 7-10 Reserved
    SVC_Handler,                          // 11 SVCall Handler
    DebugMon_Handler,                     // 12 Debug Monitor Handler
    0,                                    // 13 Reserved
    PendSV_Handler,                       // 14 PendSV Handler
    SysTick_Handler,                      // 15 SysTick Handler

    // 外设中断
    WWDG_IRQHandler,                      // 16
    PVD_IRQHandler,                       // 17
    TAMPER_IRQHandler,                    // 18
    RTC_IRQHandler,                       // 19
    FLASH_IRQHandler,                     // 20
    RCC_IRQHandler,                       // 21
    EXTI0_IRQHandler,                     // 22
    EXTI1_IRQHandler,                     // 23
    EXTI2_IRQHandler,                     // 24
    EXTI3_IRQHandler,                     // 25
    EXTI4_IRQHandler,                     // 26
    DMA1_Channel1_IRQHandler,             // 27
    DMA1_Channel2_IRQHandler,             // 28
    DMA1_Channel3_IRQHandler,             // 29
    DMA1_Channel4_IRQHandler,             // 30
    DMA1_Channel5_IRQHandler,             // 31
    DMA1_Channel6_IRQHandler,             // 32
    DMA1_Channel7_IRQHandler,             // 33
    ADC1_2_IRQHandler,                    // 34
    USB_HP_CAN1_TX_IRQHandler,            // 35
    USB_LP_CAN1_RX0_IRQHandler,           // 36
    CAN1_RX1_IRQHandler,                  // 37
    CAN1_SCE_IRQHandler,                  // 38
    EXTI9_5_IRQHandler,                   // 39
    TIM1_BRK_IRQHandler,                  // 40
    TIM1_UP_IRQHandler,                   // 41
    TIM1_TRG_COM_IRQHandler,              // 42
    TIM1_CC_IRQHandler,                   // 43
    TIM2_IRQHandler,                      // 44
    TIM3_IRQHandler,                      // 45
    TIM4_IRQHandler,                      // 46
    I2C1_EV_IRQHandler,                   // 47
    I2C1_ER_IRQHandler,                   // 48
    I2C2_EV_IRQHandler,                   // 49
    I2C2_ER_IRQHandler,                   // 50
    SPI1_IRQHandler,                      // 51
    SPI2_IRQHandler,                      // 52
    USART1_IRQHandler,                    // 53
    USART2_IRQHandler,                    // 54
    USART3_IRQHandler,                    // 55
    EXTI15_10_IRQHandler,                 // 56
    RTCAlarm_IRQHandler,                  // 57
    USBWakeUp_IRQHandler,                 // 58

    // F105/F107新增
    TIM8_BRK_IRQHandler,                  // 59
    TIM8_UP_IRQHandler,                   // 60
    TIM8_TRG_COM_IRQHandler,              // 61
    TIM8_CC_IRQHandler,                   // 62
    ADC3_IRQHandler,                      // 63
    FSMC_IRQHandler,                      // 64
    SDIO_IRQHandler,                      // 65
    TIM5_IRQHandler,                      // 66
    SPI3_IRQHandler,                      // 67
    UART4_IRQHandler,                     // 68
    UART5_IRQHandler,                     // 69
    TIM6_IRQHandler,                      // 70
    TIM7_IRQHandler,                      // 71
    DMA2_Channel1_IRQHandler,             // 72
    DMA2_Channel2_IRQHandler,             // 73
    DMA2_Channel3_IRQHandler,             // 74
    DMA2_Channel4_5_IRQHandler            // 75
};

// Reset_Handler实现
void __attribute__((naked, noreturn)) Reset_Handler(void)
{
#if defined(sram_layout) || defined(INITIALIZE_SP_AT_RESET)
    __asm ("ldr sp, =_estack");
#endif

    // 初始化.data段
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata)
        *dst++ = *src++;

    // 清零.bss段
    dst = &_sbss;
    while (dst < &_ebss)
        *dst++ = 0;
    SystemInit();
    __libc_init_array();

    main();

    for (;;);
}

void __attribute__((naked, noreturn)) Default_Handler(void)
{
    for (;;);
}