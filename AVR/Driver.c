/*********************************************
	�ļ�����Driver.c
	��  �ܣ�AVR CAFrame������ģ����� 
	
	MCU:ATMEGA328
	ʱ��:�ⲿ8MHz����
	����:WinAVR20100110
	
	о�������(http://www.chipart.cn)  ��Ȩ����
	������ҳ:http://www.chipart.cn/projects/prj_hit.asp?id=8
	
	��ʷ��¼
	о��  2019-04-14  ����
*********************************************/
#include "CAFrame.h"

uint8_t g_IntFlag;

void ser_init(void);
void sin_init(void);

/****************************************************************************
//ͨ�ýӿں���
****************************************************************************/
//1ms��ʱ�ж�
ISR(TIMER2_COMPA_vect)
{
	frm_inc_timer_tick();
}

//�豸��ʼ��
void drv_init(void)
{
	//IO�ڳ�ʼ��
	DDRB|=_BV(PB0)|_BV(PB1)|_BV(PB2);//�������
	DDRD|=_BV(PD3)|_BV(PD4)|_BV(PD5)|_BV(PD6)|_BV(PD7);//�������
	DDRC|=_BV(PC0);//LED
	
	sin_init();
	
  //���ʱ��1ms(��ʱ��2)��ʼ��
	//ʱ��:8000KHz  ��Ƶ:64 TOP:125  ����1ms 
  TCCR2A=_BV(WGM21); 
  TCCR2B=_BV(CS22);
  OCR2A=125;
  TIMSK2|=_BV(OCIE2A);
	
	//ADC��ʼ��
	//adc ʹ��,�ж�ʹ��,ʱ��:8000000/64=125KHz
	ADCSRA=_BV(ADEN)|_BV(ADIE)|_BV(ADPS1)|_BV(ADPS2);
	ADMUX=_BV(REFS0)+4;//�ڲ�AVCCΪ�ο���ѹ��AREF������ӵ���
	
	//���ڳ�ʼ��
	ser_init();
	
	//����WDT
	wdt_enable(WDTO_1S);
	wdt_reset();
	
	//���ж�����
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
//��������/���
****************************************************************************/
//������ƽӿں���
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

//��ȡ���״̬
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
	PORTC&=~_BV(PC2);//CK��ʼ�͵�ƽ
	PORTC|=_BV(PC1);//PL��ʼ�ߵ�ƽ
}
uint8_t sin_input(void)
{
	uint8_t i,dat=0;
	
	//�Ӳ�������
	IN_PORT&=~_BV(IN_PL);
	IN_PORT|=_BV(IN_PL);
	
	//���ж�ȡ
	for(i=0;i<8;i++)
	{	
		dat<<=1;
		IN_PORT|=_BV(IN_CK);
		if(IN_PIN & _BV(IN_DT))
			dat|=1;
		IN_PORT&=~_BV(IN_CK);
	}
	
	//��ȡ�ź�˳������
	i=dat&0x03;
	dat>>=4;
	i<<=4;
	dat|=i;
	return dat;
}

//��ȡ��������
uint8_t drv_input(uint8_t port)
{
	uint8_t dat=sin_input();
	
	if(dat&_BV(port))
		return 1;
	
	return 0;
}

//����״̬(ȫ��)
uint8_t drv_input_state(void)
{
	return sin_input();
}
/////////////////////////////////////////////////////////////////////////////


/****************************************************************************
//ģ������(ADC)����
****************************************************************************/
uint8_t g_AdcCounter=0;		//��ǰ�˿ڵ�ת������������
uint16_t g_AdValue[ADC_AUTO_CONVERT_TIME]; // ad ֵ�洢������

//ADת���ж� 
ISR(ADC_vect)
{
	uint16_t tmp;
	
	//���浱ǰת����ֵ
	tmp=ADCL;
	tmp|=((uint16_t)ADCH)<<8;
	g_AdValue[g_AdcCounter++]=tmp;
	
	if(g_AdcCounter < ADC_AUTO_CONVERT_TIME)//����ת����8��
		ADCSRA|=_BV(ADSC);
	else
		frm_set_event(EVENT_ADC_ID);//����ת����� 
}

//��ʼһ��ת��ת��
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
		ADCSRA &=~(_BV(ADEN)|_BV(ADSC));//ֹͣ��ǰת��
		ADCSRA |=_BV(ADEN); //����ʹ��ADC
	}
	
	ADMUX=_BV(REFS0)+ channel;				//ѡ���ʼͨ��
	g_AdcCounter=0;
	ADCSRA|=_BV(ADSC);		//��ʼת��	
}

//��ȡת�����(ƽ��ֵ)
uint8_t drv_adc_input(uint16_t *buf)
{
	uint8_t i;
	uint32_t val=0;
	
	if(g_AdcCounter < ADC_AUTO_CONVERT_TIME)//ת��������û�н���
		return 0;
		
	for(i=0;i<ADC_AUTO_CONVERT_TIME;i++)
		val+=g_AdValue[i];
	val/=ADC_AUTO_CONVERT_TIME;
	
	buf[0]=(uint16_t)val;
		
	return 1;
}
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//����(UART)����
****************************************************************************/
//�������ݽṹ
typedef struct QUEUE_S
{
	uint8_t in_index;//��ӵ�ַ
	uint8_t out_index;//���ӵ�ַ
	uint8_t buf_size; //����������
	uint8_t *pBuffer;//����
	volatile uint8_t	data_count; //���������ݸ���
	uint8_t error;
}HQUEUE,*PHQUEUE;

//����в���һ�ֽ�
void QueueInput(PHQUEUE Q,uint8_t dat)
{
	if(Q->data_count < Q->buf_size)
	{
		Q->pBuffer[Q->in_index]=dat;		//д������
		Q->in_index=(Q->in_index+1) % (Q->buf_size);//������ڵ�ַ
		Q->data_count++;	//�������ݸ���(�˲������ɱ��ж�)
	}
	else
	{
		if(Q->error<255)
			Q->error++;
	}
} 

//�Ӷ��ж���һ�ֽ�
uint8_t QueueOutput(PHQUEUE Q)
{
	uint8_t Ret=0;
	
	if(Q->data_count > 0)
	{
		Ret=Q->pBuffer[Q->out_index];	//������
		
		Q->out_index=(Q->out_index+1) % (Q->buf_size);	//�������ڵ�ַ
		Q->data_count--;
	}
	return Ret;
}
//��ö��������ݸ���
uint8_t QueueGetSize(PHQUEUE Q)
{
	return Q->data_count;
}

//��ն���,ִ��ʱ���ɱ��ж�
void QueueClear(PHQUEUE Q)
{
	drv_enter_critical();
	Q->in_index=0;
	Q->out_index=0;
	Q->data_count=0;
	Q->error=0;
	drv_exit_critical();
}

//��ʼ��һ����
void QueueCreate(PHQUEUE Q,uint8_t *buffer,uint8_t buf_size)
{
	Q->pBuffer=buffer;
	Q->buf_size=buf_size;
	QueueClear(Q);
}

#define SEL_RX_EN PORTD&=~_BV(PD2) //RS485����״̬ѡ��
#define SEL_TX_EN	PORTD|=_BV(PD2)  //RS485����״̬ѡ��

HQUEUE g_RxQueue; //�������ݶ���
HQUEUE g_TxQueue; //�������ݶ���
uint8_t g_RxBuffer[SERIAL_RECV_BUFFER_SIZE];	//���ջ���
uint8_t g_TxBuffer[SERIAL_SEND_BUFFER_SIZE];	//���ͻ���

void ser_timer_enable(void)
{
	TCNT0 = 256-((F_CPU/1000)*SERIAL_RECV_AFFIRM_TIME/1024);
	TCCR0B=_BV(CS02)|_BV(CS00);//1024��Ƶ
	TIMSK0|=_BV(TOIE0);
}
void ser_timer_disable(void)
{
	TCCR0B=0;
}

//��ʱ���ж� ����ʾ֡�������
ISR(TIMER0_OVF_vect)
{
	ser_timer_disable();	
	frm_set_event(EVENT_UART_ID);
}

void ser_init(void)
{
	//uartӲ����ʼ�� 
	UBRR0H=((F_CPU/SERIAL_BAUD/16)-1)/256;
	UBRR0L=((F_CPU/SERIAL_BAUD/16)-1)%256;
	UCSR0B=_BV(RXEN0)|_BV(TXEN0)|_BV(TXCIE0)|_BV(RXCIE0);
	UCSR0C=_BV(UCSZ01)|_BV(UCSZ00);
	
	//rs485 ���䷽�����
	DDRD|=_BV(PD2);
	SEL_RX_EN;
	
	//����/���ն��г�ʼ��
	QueueCreate(&g_RxQueue,g_RxBuffer,SERIAL_RECV_BUFFER_SIZE);
	QueueCreate(&g_TxQueue,g_TxBuffer,SERIAL_SEND_BUFFER_SIZE);
	
	//���ݰ��ж���ʱ����ֹ
	ser_timer_disable();
}

//����һ�ֽ��ж�
ISR(USART_RX_vect)
{
	uint8_t recv=UDR0;
	if(frm_get_event_state(EVENT_UART_ID))
		return ;
	QueueInput(&g_RxQueue,recv);
	ser_timer_enable();
}

//���ͼĴ������ж�
ISR(USART_UDRE_vect)
{
	if(QueueGetSize(&g_TxQueue)>0)
		UDR0=QueueOutput(&g_TxQueue);	
	else
		UCSR0B&=~_BV(UDRIE0);
}

//��������ж�
ISR(USART_TX_vect)
{
	SEL_RX_EN;
}

//����ָ�����ȵ�����
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

//������֡,���ض�ȡ����
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

//����ջ�����
void drv_serial_recv_clear(void)
{
	QueueClear(&g_RxQueue);
}
/////////////////////////////////////////////////////////////////////////////


/****************************************************************************
//EEPROM��������
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
