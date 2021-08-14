//Driver.h 硬件驱动接口函数声明文件
#ifndef DRIVER_H
#define DRIVER_H

//单片机相关头文件
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

//LED操作宏
#define LED_PORT_INIT DDRB|=_BV(PB2)
#define LED_TOGGLE PORTB^=_BV(PB2)
#define LED_SET  PORTB&=~_BV(PB2)
#define LED_CLR  PORTB|=_BV(PB2)

//输出端口符号定义
#define IO_Y1	0
#define IO_Y2 1
#define IO_Y3 2
#define IO_Y4 3
#define IO_Y5 4
#define IO_Y6 5
#define IO_Y7 6
#define IO_Y8 7

//输入端口符号定义
#define IO_X1	0
#define IO_X2 1
#define IO_X3 2
#define IO_X4 3


//模拟输入端口符号定义
#define IO_AI1 0
#define IO_AI2 1
#define IO_AI3 2

//模拟输出端口符号定义
#define IO_AO1 0
#define IO_AO2 1

//用户扩展符号定义
#define SERIAL_BAUD 9600							//UART波特率
#define SERIAL_RECV_AFFIRM_TIME 5			//接收数据包确认时间(ms) (范围:2~32)
#define SERIAL_BUFF_SIZE 64
#define SERIAL_SEND_BUFFER_SIZE  SERIAL_BUFF_SIZE		//UART发送缓冲区大小
#define SERIAL_RECV_BUFFER_SIZE  SERIAL_BUFF_SIZE		//UART接收缓冲区大小

#define ADC_AUTO_CONVERT_TIME		8  //adc每个通道一次转换次数

//用户扩展驱动接口函数声明
uint8_t drv_output_state(void);//输出状态查询
uint8_t drv_input_state(void);//输入状态查询

void drv_adc_convert(uint8_t channel);
uint8_t drv_adc_input(uint16_t *buf);

void drv_serial_output(uint8_t *buf,uint8_t len);
uint8_t drv_serial_input(uint8_t *buf);
uint8_t drv_serial_get_input_size(void);
void drv_serial_recv_clear(void); 

#endif
