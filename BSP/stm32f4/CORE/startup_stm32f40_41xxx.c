/**
 ******************************************************************************
 * @file      startup_stm32f40_41xxx.c
 * @brief     STM32F40xxx/41xxx Devices startup file for GCC toolchain
 *            C语言实现版本
 *
 *            This module performs:
 *                - Set the initial SP
 *                - Set the initial PC == Reset_Handler
 *                - Set the vector table entries with the exceptions ISR address
 *                - Branches to main in the C library (which eventually calls main())
 *
 *            After Reset the Cortex-M4 processor is in Thread mode,
 *            priority is Privileged, and the Stack is set to Main.
 ******************************************************************************
 */

#include <stdint.h>

/* 链接脚本中定义的符号 */
extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;

/* 函数声明 */
void Reset_Handler(void);
void Default_Handler(void);
void main(void);
void SystemInit(void);
void __libc_init_array(void);
int df_framework_init(void); /* 驱动框架自动初始化 */

/* Cortex-M4 系统中断 - 弱定义 */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* STM32F40x/41x 外设中断 - 弱定义 */
void WWDG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PVD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TAMP_STAMP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ADC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_RX0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_RX1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN1_SCE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_TIM9_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_TIM10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_TIM11_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_Alarm_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_FS_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_BRK_TIM12_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_UP_TIM13_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_TRG_COM_TIM14_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM8_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FSMC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SDIO_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void UART5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM6_DAC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ETH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ETH_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN2_TX_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN2_RX0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN2_RX1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CAN2_SCE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_FS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA2_Stream7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_HS_EP1_OUT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_HS_EP1_IN_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_HS_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void OTG_HS_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DCMI_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void CRYP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void HASH_RNG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FPU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

/**
 * @brief  向量表
 *         放置在 .isr_vector 段，由链接脚本定位到 0x08000000
 */
__attribute__((section(".isr_vector"), used)) void (*const g_pfnVectors[])(void) = {
    /* Cortex-M4 系统中断 */
    (void (*)(void))(&_estack), // 0  初始栈指针
    Reset_Handler,              // 1  复位处理
    NMI_Handler,                // 2  NMI
    HardFault_Handler,          // 3  硬件错误
    MemManage_Handler,          // 4  内存管理错误
    BusFault_Handler,           // 5  总线错误
    UsageFault_Handler,         // 6  使用错误
    0, 0, 0, 0,                 // 7-10 保留
    SVC_Handler,                // 11 SVCall
    DebugMon_Handler,           // 12 调试监视器
    0,                          // 13 保留
    PendSV_Handler,             // 14 PendSV
    SysTick_Handler,            // 15 SysTick

    /* STM32F40x/41x 外设中断 */
    WWDG_IRQHandler,               // 16 Window WatchDog
    PVD_IRQHandler,                // 17 PVD through EXTI Line detection
    TAMP_STAMP_IRQHandler,         // 18 Tamper and TimeStamps through EXTI line
    RTC_WKUP_IRQHandler,           // 19 RTC Wakeup through EXTI line
    FLASH_IRQHandler,              // 20 FLASH
    RCC_IRQHandler,                // 21 RCC
    EXTI0_IRQHandler,              // 22 EXTI Line0
    EXTI1_IRQHandler,              // 23 EXTI Line1
    EXTI2_IRQHandler,              // 24 EXTI Line2
    EXTI3_IRQHandler,              // 25 EXTI Line3
    EXTI4_IRQHandler,              // 26 EXTI Line4
    DMA1_Stream0_IRQHandler,       // 27 DMA1 Stream 0
    DMA1_Stream1_IRQHandler,       // 28 DMA1 Stream 1
    DMA1_Stream2_IRQHandler,       // 29 DMA1 Stream 2
    DMA1_Stream3_IRQHandler,       // 30 DMA1 Stream 3
    DMA1_Stream4_IRQHandler,       // 31 DMA1 Stream 4
    DMA1_Stream5_IRQHandler,       // 32 DMA1 Stream 5
    DMA1_Stream6_IRQHandler,       // 33 DMA1 Stream 6
    ADC_IRQHandler,                // 34 ADC1, ADC2 and ADC3s
    CAN1_TX_IRQHandler,            // 35 CAN1 TX
    CAN1_RX0_IRQHandler,           // 36 CAN1 RX0
    CAN1_RX1_IRQHandler,           // 37 CAN1 RX1
    CAN1_SCE_IRQHandler,           // 38 CAN1 SCE
    EXTI9_5_IRQHandler,            // 39 External Line[9:5]s
    TIM1_BRK_TIM9_IRQHandler,      // 40 TIM1 Break and TIM9
    TIM1_UP_TIM10_IRQHandler,      // 41 TIM1 Update and TIM10
    TIM1_TRG_COM_TIM11_IRQHandler, // 42 TIM1 Trigger and Commutation and TIM11
    TIM1_CC_IRQHandler,            // 43 TIM1 Capture Compare
    TIM2_IRQHandler,               // 44 TIM2
    TIM3_IRQHandler,               // 45 TIM3
    TIM4_IRQHandler,               // 46 TIM4
    I2C1_EV_IRQHandler,            // 47 I2C1 Event
    I2C1_ER_IRQHandler,            // 48 I2C1 Error
    I2C2_EV_IRQHandler,            // 49 I2C2 Event
    I2C2_ER_IRQHandler,            // 50 I2C2 Error
    SPI1_IRQHandler,               // 51 SPI1
    SPI2_IRQHandler,               // 52 SPI2
    USART1_IRQHandler,             // 53 USART1
    USART2_IRQHandler,             // 54 USART2
    USART3_IRQHandler,             // 55 USART3
    EXTI15_10_IRQHandler,          // 56 External Line[15:10]s
    RTC_Alarm_IRQHandler,          // 57 RTC Alarm (A and B) through EXTI Line
    OTG_FS_WKUP_IRQHandler,        // 58 USB OTG FS Wakeup through EXTI line
    TIM8_BRK_TIM12_IRQHandler,     // 59 TIM8 Break and TIM12
    TIM8_UP_TIM13_IRQHandler,      // 60 TIM8 Update and TIM13
    TIM8_TRG_COM_TIM14_IRQHandler, // 61 TIM8 Trigger and Commutation and TIM14
    TIM8_CC_IRQHandler,            // 62 TIM8 Capture Compare
    DMA1_Stream7_IRQHandler,       // 63 DMA1 Stream7
    FSMC_IRQHandler,               // 64 FSMC
    SDIO_IRQHandler,               // 65 SDIO
    TIM5_IRQHandler,               // 66 TIM5
    SPI3_IRQHandler,               // 67 SPI3
    UART4_IRQHandler,              // 68 UART4
    UART5_IRQHandler,              // 69 UART5
    TIM6_DAC_IRQHandler,           // 70 TIM6 and DAC1&2 underrun errors
    TIM7_IRQHandler,               // 71 TIM7
    DMA2_Stream0_IRQHandler,       // 72 DMA2 Stream 0
    DMA2_Stream1_IRQHandler,       // 73 DMA2 Stream 1
    DMA2_Stream2_IRQHandler,       // 74 DMA2 Stream 2
    DMA2_Stream3_IRQHandler,       // 75 DMA2 Stream 3
    DMA2_Stream4_IRQHandler,       // 76 DMA2 Stream 4
    ETH_IRQHandler,                // 77 Ethernet
    ETH_WKUP_IRQHandler,           // 78 Ethernet Wakeup through EXTI line
    CAN2_TX_IRQHandler,            // 79 CAN2 TX
    CAN2_RX0_IRQHandler,           // 80 CAN2 RX0
    CAN2_RX1_IRQHandler,           // 81 CAN2 RX1
    CAN2_SCE_IRQHandler,           // 82 CAN2 SCE
    OTG_FS_IRQHandler,             // 83 USB OTG FS
    DMA2_Stream5_IRQHandler,       // 84 DMA2 Stream 5
    DMA2_Stream6_IRQHandler,       // 85 DMA2 Stream 6
    DMA2_Stream7_IRQHandler,       // 86 DMA2 Stream 7
    USART6_IRQHandler,             // 87 USART6
    I2C3_EV_IRQHandler,            // 88 I2C3 event
    I2C3_ER_IRQHandler,            // 89 I2C3 error
    OTG_HS_EP1_OUT_IRQHandler,     // 90 USB OTG HS End Point 1 Out
    OTG_HS_EP1_IN_IRQHandler,      // 91 USB OTG HS End Point 1 In
    OTG_HS_WKUP_IRQHandler,        // 92 USB OTG HS Wakeup through EXTI
    OTG_HS_IRQHandler,             // 93 USB OTG HS
    DCMI_IRQHandler,               // 94 DCMI
    CRYP_IRQHandler,               // 95 CRYP crypto
    HASH_RNG_IRQHandler,           // 96 Hash and Rng
    FPU_IRQHandler                 // 97 FPU
};

/**
 * @brief  复位处理函数
 *         - 初始化 .data 段（从Flash复制到RAM）
 *         - 清零 .bss 段
 *         - 调用 SystemInit() 配置时钟
 *         - 调用 __libc_init_array() 初始化C库
 *         - 跳转到 main()
 */
void __attribute__((naked, noreturn)) Reset_Handler(void)
{
#if defined(sram_layout) || defined(INITIALIZE_SP_AT_RESET)
    __asm("ldr sp, =_estack");
#endif

    /* 初始化 .data 段：从Flash复制初始化数据到RAM */
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;
    while (dst < &_edata)
        *dst++ = *src++;

    /* 清零 .bss 段 */
    dst = &_sbss;
    while (dst < &_ebss)
        *dst++ = 0;

    /* 调用系统初始化函数 */
    SystemInit();

    /* 调用C库初始化 */
    __libc_init_array();

    /* 调用驱动框架自动初始化 */
    df_framework_init();

    /* 跳转到main函数 */
    main();

    /* 如果main返回，进入死循环 */
    for (;;)
        ;
}

/**
 * @brief  默认中断处理函数
 *         未实现的中断会跳转到这里，进入死循环
 */
void __attribute__((naked, noreturn)) Default_Handler(void)
{
    for (;;)
        ;
}
