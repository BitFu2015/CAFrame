//Driver.h Ӳ�������ӿں��������ļ�
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

//LED������
#define LED_ON  GPIO_SetBits(GPIOB , GPIO_Pin_12)
#define LED_OFF GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define LED_TOGGLE 	GPIOB->ODR ^= GPIO_Pin_12;

//����˿ڷ��Ŷ���
#define IO_Y1	0
#define IO_Y2 1
#define IO_Y3 2
#define IO_Y4 3
#define IO_Y5 4
#define IO_Y6 5
#define IO_Y7	6
#define IO_Y8 7

//����˿ڷ��Ŷ���
#define IO_X1	0
#define IO_X2 1
#define IO_X3 2
#define IO_X4 3

//ģ������˿ڷ��Ŷ���
#define IO_AI1 0
#define IO_AI2 1

//ģ������˿ڷ��Ŷ���
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
uint32_t GetCurrentTick(void);  //��ȡ��ǰ��ʱֵ
uint32_t GetPassedTick(uint32_t t);	 //�����ȥʱ��
void SysDelayMs(uint32_t t);	 //��ʱ
void DelayMs(uint32_t t);	 //������ʱ���뺯��(ϵͳʱ�ӣ�72MHz)
void DelayUs(uint32_t t);	//������ʱ΢�뺯��(ϵͳʱ��:72MHz)

#endif
