//Driver.h Ӳ�������ӿں��������ļ�
#ifndef DRIVER_H
#define DRIVER_H

#include "stm8s.h"
#include "queue.h"
#include "fm1702.h"
#include "string.h"

#define MY_DEBUG 0
#define WDT_ENABLE 1

#define CLI 	disableInterrupts
#define SEI	  enableInterrupts

#if MY_DEBUG>0
	#include "stdio.h"
	#define PUTCHAR_PROTOTYPE char putchar (char c)
	#define GETCHAR_PROTOTYPE char getchar (void)
	void UART1_Init(void);
	void UartSend(uint8_t *buf,uint8_t size);
	void UartPutc(uint8_t c);
#endif

#define SERIAL_BAUD 9600							//UART������
#define SERIAL_RECV_AFFIRM_TIME 5			//�������ݰ�ȷ��ʱ��(ms) (��Χ:2~32)
#define SERIAL_BUFF_SIZE 64
#define SERIAL_SEND_BUFFER_SIZE  SERIAL_BUFF_SIZE		//UART���ͻ�������С
#define SERIAL_RECV_BUFFER_SIZE  SERIAL_BUFF_SIZE		//UART���ջ�������С

//driver.c
void DelayMs(uint16_t ms);
void ser_init(void);
void drv_serial_output(uint8_t *buf,uint8_t len);
uint8_t drv_serial_input(uint8_t *buf);
uint8_t drv_serial_get_input_size(void);
void drv_serial_recv_clear(void);

//LED������
#define LED_TOGGLE GPIOD->ODR^=(1<<2)//����ܿ���ȡ������
#define LED_OFF  GPIOD->ODR|=(1<<2)//�������
#define LED_ON  GPIOD->ODR&=~(1<<2)//�������

#define LED_POWER_TOGGLE GPIOD->ODR^=(1<<3)
#define LED_POWER_ON	GPIOD->ODR&=~(1<<3)
#define LED_POWER_OFF GPIOD->ODR|=(1<<3)

#define BEEP_ON	GPIOD->ODR|=(1<<4)
#define BEEP_OFF GPIOD->ODR&=~(1<<4)

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

//ģ������˿ڷ��Ŷ���
#define IO_AO1 0
#define IO_AO2 1

#endif
