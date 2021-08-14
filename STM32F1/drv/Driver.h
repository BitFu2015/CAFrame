//Driver.h 硬件驱动接口函数声明文件
#ifndef DRIVER_H
#define DRIVER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stm32f10x.h"
#include "queue.h"
#include "serial.h"
#include "24cxx.h"
#include "adc.h"


#define CLI()  __set_PRIMASK(1)  
#define SEI()  __set_PRIMASK(0) 

#define DEFAULT_INT_PRIORITY 15

//LED操作宏
#define LED_ON  GPIO_SetBits(GPIOB , GPIO_Pin_12)
#define LED_OFF GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define LED_TOGGLE 	GPIOB->ODR ^= GPIO_Pin_12;

//输出端口符号定义
#define IO_Y1	0
#define IO_Y2 1
#define IO_Y3 2
#define IO_Y4 3
#define IO_Y5 4
#define IO_Y6 5
#define IO_Y7	6
#define IO_Y8 7

//输入端口符号定义
#define IO_X1	0
#define IO_X2 1
#define IO_X3 2
#define IO_X4 3

//模拟输入端口符号定义
#define IO_AI1 0
#define IO_AI2 1

//模拟输出端口符号定义
#define IO_AO1 0
#define IO_AO2 1

#define drv_eeprom_read eep_read
#define drv_eeprom_write eep_write

//driver.c
uint8_t drv_output_status(uint8_t port);

void DebugPrintInit(uint32_t ulWantedBaud);
void DebugPrintBin(uint8_t *buf,uint8_t len);
	
//timer.c 
void InitTick(void);
uint32_t GetCurrentTick(void);  //获取当前定时值
uint32_t GetPassedTick(uint32_t t);	 //计算过去时间
void SysDelayMs(uint32_t t);	 //延时
void DelayMs(uint32_t t);	 //粗略延时毫秒函数(系统时钟：72MHz)
void DelayUs(uint32_t t);	//粗略延时微秒函数(系统时钟:72MHz)

#endif
