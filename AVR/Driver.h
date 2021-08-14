//Driver.h Ӳ�������ӿں��������ļ�
#ifndef DRIVER_H
#define DRIVER_H

//��Ƭ�����ͷ�ļ�
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

//LED������
#define LED_PORT_INIT DDRB|=_BV(PB2)
#define LED_TOGGLE PORTB^=_BV(PB2)
#define LED_SET  PORTB&=~_BV(PB2)
#define LED_CLR  PORTB|=_BV(PB2)

//����˿ڷ��Ŷ���
#define IO_Y1	0
#define IO_Y2 1
#define IO_Y3 2
#define IO_Y4 3
#define IO_Y5 4
#define IO_Y6 5
#define IO_Y7 6
#define IO_Y8 7

//����˿ڷ��Ŷ���
#define IO_X1	0
#define IO_X2 1
#define IO_X3 2
#define IO_X4 3


//ģ������˿ڷ��Ŷ���
#define IO_AI1 0
#define IO_AI2 1
#define IO_AI3 2

//ģ������˿ڷ��Ŷ���
#define IO_AO1 0
#define IO_AO2 1

//�û���չ���Ŷ���
#define SERIAL_BAUD 9600							//UART������
#define SERIAL_RECV_AFFIRM_TIME 5			//�������ݰ�ȷ��ʱ��(ms) (��Χ:2~32)
#define SERIAL_BUFF_SIZE 64
#define SERIAL_SEND_BUFFER_SIZE  SERIAL_BUFF_SIZE		//UART���ͻ�������С
#define SERIAL_RECV_BUFFER_SIZE  SERIAL_BUFF_SIZE		//UART���ջ�������С

#define ADC_AUTO_CONVERT_TIME		8  //adcÿ��ͨ��һ��ת������

//�û���չ�����ӿں�������
uint8_t drv_output_state(void);//���״̬��ѯ
uint8_t drv_input_state(void);//����״̬��ѯ

void drv_adc_convert(uint8_t channel);
uint8_t drv_adc_input(uint16_t *buf);

void drv_serial_output(uint8_t *buf,uint8_t len);
uint8_t drv_serial_input(uint8_t *buf);
uint8_t drv_serial_get_input_size(void);
void drv_serial_recv_clear(void); 

#endif
