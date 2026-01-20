/**
 ******************************************************************************
 * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
 * @author  MCD Application Team
 * @version V3.6.0
 * @date    20-September-2021
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2011 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include <stdio.h>

/** @addtogroup STM32F10x_StdPeriph_Template
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
  volatile uint32_t g_hfsr, g_cfsr, g_bfar, g_mmfar;
  volatile uint32_t g_stacked_r0, g_stacked_r1, g_stacked_r2, g_stacked_r3, g_stacked_r12, g_stacked_pc, g_stacked_lr, g_stacked_psr, g_stacked_sp;

  /* Go to infinite loop when Hard Fault exception occurs */
  uint32_t *sp = (uint32_t *)__get_MSP(); // 或根据是否在中断中选择 MSP/PSP
  g_hfsr = SCB->HFSR;
  g_cfsr = SCB->CFSR;
  g_bfar = SCB->BFAR;
  g_mmfar = SCB->MMFAR;

  if ((g_cfsr & 0x00000002) != 0)
    g_mmfar = *(uint32_t *)g_mmfar;
  if ((g_cfsr & 0x00000080) != 0)
    g_bfar = *(uint32_t *)g_bfar;
  if ((g_cfsr & 0x00008000) != 0)
    g_bfar = *(uint32_t *)g_bfar;
  if ((g_cfsr & 0x02000000) != 0)
    g_mmfar = *(uint32_t *)g_mmfar;

  g_stacked_r0 = sp[0];
  g_stacked_r1 = sp[1];
  g_stacked_r2 = sp[2];
  g_stacked_r3 = sp[3];
  g_stacked_r12 = sp[4];
  g_stacked_lr = sp[5];
  g_stacked_pc = sp[6];
  g_stacked_psr = sp[7];
  g_stacked_sp = (uint32_t)sp;

  extern int log_flush(void);
  log_flush();
  printf("\n=== HardFault Detected ===\n");
  printf("HFSR:  0x%08X\n", g_hfsr);
  printf("CFSR:  0x%08X\n", g_cfsr);
  printf("BFAR:  0x%08X\n", g_bfar);
  printf("MMFAR: 0x%08X\n", g_mmfar);
  printf("PC:    0x%08X\n", g_stacked_pc);
  printf("LR:    0x%08X\n", g_stacked_lr);

  while (1)
  {
  }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
}

// /**
//   * @brief  This function handles SysTick Handler.
//   * @param  None
//   * @retval None
//   */
// void SysTick_Handler(void)
// {
// 	SysTick_CallBack();
// }

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/

/**
 * @}
 */
