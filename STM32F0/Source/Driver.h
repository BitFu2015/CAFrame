//Driver.h Ӳ�������ӿں��������ļ�

#ifndef DRIVER_H
#define DRIVER_H

#define DEBUG_MODE      //����ģʽ,�����ʹ��printf����ע�ʹ˶���


//ͷ�ļ�����///////////////////////////////////////////////
#include "stm32f0xx.h"

#define LED_ON	GPIOC->BSRR=GPIO_Pin_15
#define LED_OFF	GPIOC->BRR=GPIO_Pin_15
#define LED_TOGGLE GPIOC->ODR^=GPIO_Pin_15

#define RS485_TX_SEL GPIOA->BSRR=GPIO_Pin_8
#define RS485_RX_SEL GPIOA->BRR=GPIO_Pin_8

#ifdef DEBUG_MODE
	#include <stdio.h>
	void DebugPrintInit(void);
#endif


//IO������////////////////////////////////////////////////

//����˿ڷ��Ŷ���
#define IO_Y1	0
#define IO_Y2 1
#define IO_Y3 2
#define IO_Y4 3
#define IO_Y5 4
#define IO_Y6 5

//����˿ڷ��Ŷ���
#define IO_X1	0
#define IO_X2 1
#define IO_X3 2
#define IO_X4 3
#define IO_X5 4
#define IO_X6 5

//ģ������˿ڷ��Ŷ���
#define IO_AI1 0
#define IO_AI2 1
#define IO_AI3 2

//ģ������˿ڷ��Ŷ���
#define IO_AO1 0
#define IO_AO2 1

#endif
