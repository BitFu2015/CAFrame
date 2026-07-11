//driver.h
#ifndef DRIVER_H
#define DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

//LL LIB
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_exti.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_pwr.h"
#include "stm32g0xx_ll_dma.h"
//#include "stm32g0xx_ll_spi.h"
#include "stm32g0xx_ll_adc.h"
#include "stm32g0xx_ll_usart.h"
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_iwdg.h"

//c lib
#include <string.h>
#include <stdio.h>

//user driver
#include "flash.h"
#include "veep.h"

#define CLI()  __set_PRIMASK(1)  
#define SEI()  __set_PRIMASK(0) 

//LED 
#define LED_ON	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4)
#define LED_OFF  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4)
#define LED_TOGGLE LL_GPIO_TogglePin(GPIOA,LL_GPIO_PIN_4)


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

//用户扩展驱动接口函数
void drv_sys_reset(void);
void drv_delay_us(uint32_t t);
void drv_delay_ms(uint32_t t);

//串口驱动缓冲区大小定义
#define SERIAL_BUFF_SIZE 256

#define UART_RX_IDLE 2
#define UART_RX_BUSY 1
#define UART_RX_READY 0

//UART1
uint32_t drv_serial_output(uint8_t *buf,uint32_t len);
uint32_t drv_serial_input(uint8_t *buf);

//UART2
uint32_t Serial2Write(uint8_t *buf,uint32_t len);
uint32_t Serial2Read(uint8_t *buf);


#ifdef __cplusplus
}
#endif

#endif
