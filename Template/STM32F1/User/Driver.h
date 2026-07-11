//Driver.h 硬件驱动接口函数声明文件
#ifndef DRIVER_H
#define DRIVER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_cortex.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_pwr.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_iwdg.h"
#include "stm32f1xx_ll_adc.h"

#define CLI()  __set_PRIMASK(1)  
#define SEI()  __set_PRIMASK(0) 

//Private defines 
#ifndef NVIC_PRIORITYGROUP_0
	#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority, 4 bits for subpriority */
	#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority, 3 bits for subpriority */
	#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority, 2 bits for subpriority */
	#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority, 1 bit  for subpriority */
	#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority, 0 bit  for subpriority */
#endif

//自用中断优先级定义
#define PRIORITY_H 1
#define PRIORITY_M 10
#define PRIORITY_L 15
#define PRIORITY_DEF 14
#define PRIORITY_SUB 0   //响应优先级固定值定义

#define LED_ON  LL_GPIO_SetOutputPin(GPIOB , LL_GPIO_PIN_2)
#define LED_OFF LL_GPIO_ResetOutputPin(GPIOB,LL_GPIO_PIN_2)
#define LED_TOGGLE 	GPIOB->ODR^=LL_GPIO_PIN_2

//输出端口符号定义
#define IO_Y1	0
#define IO_Y2 1
#define IO_Y3 2
#define IO_Y4 3
#define IO_Y5 4
#define IO_Y6 5
#define IO_Y7	6
#define IO_Y8 7

#define IO_YJ 	IO_Y1	//压机
#define IO_DR 	IO_Y2	//电热
#define IO_ST 	IO_Y3	//四通阀
#define IO_FJ 	IO_Y4 //风机
#define IO_JS 	IO_Y5 //加湿
#define IO_MC		IO_Y6 //脉冲
#define IO_DC		IO_Y7 //等离子
#define IO_ZW 	IO_Y8 //紫外灯

//输入端口符号定义
#define IO_X1	0
#define IO_X2 1
#define IO_X3 2
#define IO_X4 3

//模拟输入端口符号定义
#define IO_AI1 0
#define IO_AI2 1
#define IO_AI3 2
#define IO_AI4 3

//模拟输出端口符号定义
#define IO_AO1 0
#define IO_AO2 1


#include "serial.h"
#include "24cxx.h"

#endif
