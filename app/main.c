#include "main.h"
#include "shell_style.h"

int main(){
	Systick_Init(); // 初始化SysTick定时器
	LED_Init(); // 初始化LED
	NVIC_Init(); // 初始化中断
	USART1_Init(250000);
	MCU_Shell_Init(&Shell, &STM32F103C8T6_Device); // 初始化Shell
	while(1){
		Task_Switch_Tick_Handler(&Shell_Sysfpoint);
	}
	return 0;
}



