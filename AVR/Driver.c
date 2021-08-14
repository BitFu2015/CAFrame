/*********************************************
	文件名：Driver.c
	功  能：AVR CAFrame驱动层模板程序 
	
	MCU:ATMEGA328
	时钟:外部8MHz晶振
	编译:WinAVR20100110
	
	芯艺设计室(http://www.chipart.cn)  版权所有
	工程主页:http://www.chipart.cn/projects/prj_hit.asp?id=8
	
	历史记录
	芯艺  2019-04-14  创建
*********************************************/
#include "CAFrame.h"

uint8_t g_IntFlag;

void ser_init(void);
void sin_init(void);

/****************************************************************************
//通用接口函数
****************************************************************************/
//1ms定时中断
ISR(TIMER2_COMPA_vect)
{
	frm_inc_timer_tick();
}

//设备初始化
void drv_init(void)
{
	//IO口初始化
	DDRB|=_BV(PB0)|_BV(PB1)|_BV(PB2);//输出控制
	DDRD|=_BV(PD3)|_BV(PD4)|_BV(PD5)|_BV(PD6)|_BV(PD7);//输出控制
	DDRC|=_BV(PC0);//LED
	
	sin_init();
	
  //框架时钟1ms(定时器2)初始化
	//时钟:8000KHz  分频:64 TOP:125  周期1ms 
  TCCR2A=_BV(WGM21); 
  TCCR2B=_BV(CS22);
  OCR2A=125;
  TIMSK2|=_BV(OCIE2A);
	
	//ADC初始化
	//adc 使能,中断使能,时钟:8000000/64=125KHz
	ADCSRA=_BV(ADEN)|_BV(ADIE)|_BV(ADPS1)|_BV(ADPS2);
	ADMUX=_BV(REFS0)+4;//内部AVCC为参考电压，AREF引脚外接电容
	
	//串口初始化
	ser_init();
	
	//启用WDT
	wdt_enable(WDTO_1S);
	wdt_reset();
	
	//总中断允许
	sei();
}

void drv_enter_critical(void)
{
	g_IntFlag=SREG&0x80;
	cli();
}

void drv_exit_critical(void)
{
	if(g_IntFlag==0x80)
		sei();
}

void drv_wdt_reset(void)
{
	wdt_reset();
}
/////////////////////////////////////////////////////////////////////////////


/****************************************************************************
//开关输入/输出
****************************************************************************/
//输出控制接口函数
void drv_output(uint8_t port,uint8_t val)
{
	if(val)
	{
		if(port==IO_Y1)
			PORTB|=_BV(PB2);
		else if(port==IO_Y2)
			PORTB|=_BV(PB1);
		else if(port==IO_Y3)
			PORTB|=_BV(PB0);
		else if(port==IO_Y4)
			PORTD|=_BV(PD7);
		else if(port==IO_Y5)
			PORTD|=_BV(PD6);
		else if(port==IO_Y6)
			PORTD|=_BV(PD5);
		else if(port==IO_Y7)
			PORTD|=_BV(PD4);
		else if(port==IO_Y8)
			PORTD|=_BV(PD3);
		else;
	}
	else
	{
		if(port==IO_Y1)
			PORTB&=~_BV(PB2);
		else if(port==IO_Y2)
			PORTB&=~_BV(PB1);
		else if(port==IO_Y3)
			PORTB&=~_BV(PB0);
		else if(port==IO_Y4)
			PORTD&=~_BV(PD7);
		else if(port==IO_Y5)
			PORTD&=~_BV(PD6);
		else if(port==IO_Y6)
			PORTD&=~_BV(PD5);
		else if(port==IO_Y7)
			PORTD&=~_BV(PD4);
		else if(port==IO_Y8)
			PORTD&=~_BV(PD3);
		else;		
	}
}

//获取输出状态
uint8_t drv_output_state(void)
{
	uint8_t ret=0;
	uint8_t i,tmp;
	
	tmp=PORTD;
	for(i=0;i<5;i++)
	{
		ret<<=1;
		if(tmp & _BV(PD3))
			ret|=1;
		tmp>>=1;
	}
	tmp=PORTB;
	for(i=0;i<3;i++)
	{
		ret<<=1;
		if(tmp&0x01)
			ret|=1;
		tmp>>=1;
	}
	return ret;
}
#define IN_DT PC3
#define IN_CK PC2
#define IN_PL PC1
#define IN_PORT PORTC
#define IN_PIN PINC
void sin_init(void)
{
	DDRC|=_BV(PC1)|_BV(PC2);
	DDRC&=~_BV(PC3);
	PORTC&=~_BV(PC2);//CK初始低电平
	PORTC|=_BV(PC1);//PL初始高电平
}
uint8_t sin_input(void)
{
	uint8_t i,dat=0;
	
	//从并口锁存
	IN_PORT&=~_BV(IN_PL);
	IN_PORT|=_BV(IN_PL);
	
	//串行读取
	for(i=0;i<8;i++)
	{	
		dat<<=1;
		IN_PORT|=_BV(IN_CK);
		if(IN_PIN & _BV(IN_DT))
			dat|=1;
		IN_PORT&=~_BV(IN_CK);
	}
	
	//读取信号顺序修正
	i=dat&0x03;
	dat>>=4;
	i<<=4;
	dat|=i;
	return dat;
}

//获取输入检测结果
uint8_t drv_input(uint8_t port)
{
	uint8_t dat=sin_input();
	
	if(dat&_BV(port))
		return 1;
	
	return 0;
}

//输入状态(全部)
uint8_t drv_input_state(void)
{
	return sin_input();
}
/////////////////////////////////////////////////////////////////////////////


/****************************************************************************
//模拟输入(ADC)驱动
****************************************************************************/
uint8_t g_AdcCounter=0;		//当前端口的转换次数计数器
uint16_t g_AdValue[ADC_AUTO_CONVERT_TIME]; // ad 值存储缓冲区

//AD转换中断 
ISR(ADC_vect)
{
	uint16_t tmp;
	
	//保存当前转换的值
	tmp=ADCL;
	tmp|=((uint16_t)ADCH)<<8;
	g_AdValue[g_AdcCounter++]=tmp;
	
	if(g_AdcCounter < ADC_AUTO_CONVERT_TIME)//继续转换至8次
		ADCSRA|=_BV(ADSC);
	else
		frm_set_event(EVENT_ADC_ID);//本次转换完成 
}

//开始一次转换转换
void drv_adc_convert(uint8_t channel)
{
	if(channel ==IO_AI1)
		channel=4;
	else if(channel == IO_AI2)
		channel=5;
	else
		channel=7;
		
	if(frm_get_event_state(EVENT_ADC_ID)==0)
	{
		ADCSRA &=~(_BV(ADEN)|_BV(ADSC));//停止当前转换
		ADCSRA |=_BV(ADEN); //重新使能ADC
	}
	
	ADMUX=_BV(REFS0)+ channel;				//选择初始通道
	g_AdcCounter=0;
	ADCSRA|=_BV(ADSC);		//开始转换	
}

//读取转换结果(平均值)
uint8_t drv_adc_input(uint16_t *buf)
{
	uint8_t i;
	uint32_t val=0;
	
	if(g_AdcCounter < ADC_AUTO_CONVERT_TIME)//转换操作还没有结束
		return 0;
		
	for(i=0;i<ADC_AUTO_CONVERT_TIME;i++)
		val+=g_AdValue[i];
	val/=ADC_AUTO_CONVERT_TIME;
	
	buf[0]=(uint16_t)val;
		
	return 1;
}
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//串口(UART)驱动
****************************************************************************/
//队列数据结构
typedef struct QUEUE_S
{
	uint8_t in_index;//入队地址
	uint8_t out_index;//出队地址
	uint8_t buf_size; //缓冲区长度
	uint8_t *pBuffer;//缓冲
	volatile uint8_t	data_count; //队列内数据个数
	uint8_t error;
}HQUEUE,*PHQUEUE;

//向队列插入一字节
void QueueInput(PHQUEUE Q,uint8_t dat)
{
	if(Q->data_count < Q->buf_size)
	{
		Q->pBuffer[Q->in_index]=dat;		//写入数据
		Q->in_index=(Q->in_index+1) % (Q->buf_size);//调整入口地址
		Q->data_count++;	//调整数据个数(此操作不可被中断)
	}
	else
	{
		if(Q->error<255)
			Q->error++;
	}
} 

//从队列读出一字节
uint8_t QueueOutput(PHQUEUE Q)
{
	uint8_t Ret=0;
	
	if(Q->data_count > 0)
	{
		Ret=Q->pBuffer[Q->out_index];	//读数据
		
		Q->out_index=(Q->out_index+1) % (Q->buf_size);	//调整出口地址
		Q->data_count--;
	}
	return Ret;
}
//获得队列中数据个数
uint8_t QueueGetSize(PHQUEUE Q)
{
	return Q->data_count;
}

//清空队列,执行时不可被中断
void QueueClear(PHQUEUE Q)
{
	drv_enter_critical();
	Q->in_index=0;
	Q->out_index=0;
	Q->data_count=0;
	Q->error=0;
	drv_exit_critical();
}

//初始化一队列
void QueueCreate(PHQUEUE Q,uint8_t *buffer,uint8_t buf_size)
{
	Q->pBuffer=buffer;
	Q->buf_size=buf_size;
	QueueClear(Q);
}

#define SEL_RX_EN PORTD&=~_BV(PD2) //RS485接收状态选择
#define SEL_TX_EN	PORTD|=_BV(PD2)  //RS485发送状态选择

HQUEUE g_RxQueue; //接收数据队列
HQUEUE g_TxQueue; //发送数据队列
uint8_t g_RxBuffer[SERIAL_RECV_BUFFER_SIZE];	//接收缓冲
uint8_t g_TxBuffer[SERIAL_SEND_BUFFER_SIZE];	//发送缓冲

void ser_timer_enable(void)
{
	TCNT0 = 256-((F_CPU/1000)*SERIAL_RECV_AFFIRM_TIME/1024);
	TCCR0B=_BV(CS02)|_BV(CS00);//1024分频
	TIMSK0|=_BV(TOIE0);
}
void ser_timer_disable(void)
{
	TCCR0B=0;
}

//定时器中断 ，表示帧接收完成
ISR(TIMER0_OVF_vect)
{
	ser_timer_disable();	
	frm_set_event(EVENT_UART_ID);
}

void ser_init(void)
{
	//uart硬件初始化 
	UBRR0H=((F_CPU/SERIAL_BAUD/16)-1)/256;
	UBRR0L=((F_CPU/SERIAL_BAUD/16)-1)%256;
	UCSR0B=_BV(RXEN0)|_BV(TXEN0)|_BV(TXCIE0)|_BV(RXCIE0);
	UCSR0C=_BV(UCSZ01)|_BV(UCSZ00);
	
	//rs485 传输方向控制
	DDRD|=_BV(PD2);
	SEL_RX_EN;
	
	//发送/接收队列初始化
	QueueCreate(&g_RxQueue,g_RxBuffer,SERIAL_RECV_BUFFER_SIZE);
	QueueCreate(&g_TxQueue,g_TxBuffer,SERIAL_SEND_BUFFER_SIZE);
	
	//数据包判定定时器禁止
	ser_timer_disable();
}

//接收一字节中断
ISR(USART_RX_vect)
{
	uint8_t recv=UDR0;
	if(frm_get_event_state(EVENT_UART_ID))
		return ;
	QueueInput(&g_RxQueue,recv);
	ser_timer_enable();
}

//发送寄存器空中断
ISR(USART_UDRE_vect)
{
	if(QueueGetSize(&g_TxQueue)>0)
		UDR0=QueueOutput(&g_TxQueue);	
	else
		UCSR0B&=~_BV(UDRIE0);
}

//发送完成中断
ISR(USART_TX_vect)
{
	SEL_RX_EN;
}

//发送指定长度的数据
void drv_serial_output(uint8_t *buf,uint8_t len)
{
	uint8_t i;
	
	QueueClear(&g_TxQueue);
	for(i=1;i<len;i++)
	{
		drv_enter_critical();
		QueueInput(&g_TxQueue,buf[i]);
		drv_exit_critical();
	}
	
	SEL_TX_EN;
	UDR0=buf[0];
	UCSR0B|=_BV(UDRIE0);			
}

//读数据帧,返回读取长度
uint8_t drv_serial_input(uint8_t *buf)
{
	uint8_t i;
	
	for(i=0;i<SERIAL_RECV_BUFFER_SIZE;i++)
	{
		if(QueueGetSize(&g_RxQueue)==0)
			break;
			
		drv_enter_critical();
		buf[i]=QueueOutput(&g_RxQueue);
		drv_exit_critical();
	}	
	QueueClear(&g_RxQueue);
	return i;
}

uint8_t drv_serial_get_input_size(void)
{
	return QueueGetSize(&g_RxQueue);
}

//清接收缓冲区
void drv_serial_recv_clear(void)
{
	QueueClear(&g_RxQueue);
}
/////////////////////////////////////////////////////////////////////////////


/****************************************************************************
//EEPROM操作驱动
****************************************************************************/
void drv_eeprom_read(uint16_t addr,uint8_t *buf,uint8_t size)
{
	uint8_t i;
	uint8_t *p=(uint8_t *)addr;
	
	wdt_reset();
	eeprom_busy_wait();
	for(i=0;i<size;i++)
		buf[i]=eeprom_read_byte(p++);
}

void drv_eeprom_write(uint16_t addr,uint8_t *buf,uint8_t size)
{
	uint8_t i;
	uint8_t *p=(uint8_t *)addr;
	
	for(i=0;i<size;i++)
	{
		wdt_reset();
		eeprom_busy_wait();
		eeprom_write_byte(p++,buf[i]);
	}
}
/////////////////////////////////////////////////////////////////////////////
