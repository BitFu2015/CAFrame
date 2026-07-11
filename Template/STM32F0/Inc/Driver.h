//Driver.h 硬件驱动接口函数声明文件

#ifndef DRIVER_H
#define DRIVER_H

//头文件包含///////////////////////////////////////////////
//#include "stm32f0xx_ll_adc.h"
//#include "stm32f0xx_ll_i2c.h"
#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
//#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_iwdg.h"

#include "iic.h"
#include "eep.h"

#define CLI()  __set_PRIMASK(1)  
#define SEI()  __set_PRIMASK(0) 

#define LED_ON  LL_GPIO_SetOutputPin(GPIOA , LL_GPIO_PIN_6)
#define LED_OFF LL_GPIO_ResetOutputPin(GPIOA,LL_GPIO_PIN_6)
#define LED_TOGGLE 	GPIOA->ODR^=LL_GPIO_PIN_6


#define SERIAL_BUFF_SIZE 128
void drv_serial_init(void);
uint8_t drv_serial_output(uint8_t *buf,uint8_t len);
uint8_t drv_serial_input(uint8_t *buf);


//IO定义区////////////////////////////////////////////////

//输出端口符号定义
#define IO_Y1	0
#define IO_Y2 1


//输入端口符号定义
#define IO_X1	0
#define IO_X2 1


//模拟输入端口符号定义
#define IO_AI1 0
#define IO_AI2 1
#define IO_AI3 2

//模拟输出端口符号定义
#define IO_AO1 0
#define IO_AO2 1

#endif
