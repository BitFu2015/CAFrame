/*********************************************
	�ļ�����Driver.c
	CAS727  CAFrame��������� 
	STM8S103F3  12M�ⲿ����
	
	о�������(http://www.chipart.cn)  ��Ȩ����
	
	��ʷ��¼
	о��  2017-6-16  ����
*********************************************/
#include "caframe.h"

/****************************************************************************
//ͨ�ýӿں���
****************************************************************************/

static volatile uint16_t g_DelayTimer;

void DelayMs(uint16_t ms)
{ 
	disableInterrupts();
	g_DelayTimer=ms;
	enableInterrupts();
		
	while(ms>0)
	{
		ms=g_DelayTimer;
		if(ms%100==0)
			IWDG->KR=0XAA;//��λ���Ź�
	}
} 

//1ms��ʱ�ж�
INTERRUPT void TIM1_IRQHandler (void)
{
	TIM1->SR1 =0;
	
	frm_inc_timer_tick(); //caframeʱ��
	
	if(g_DelayTimer)
		g_DelayTimer--;
}

//1msϵͳ��ʱ��
void Timer1Open(void)
{
	//���ʱ��1ms(��ʱ��1)��ʼ��			
  CLK->PCKENR1 |= (1<<7);//TIM1��ʱ��ʱ������
  TIM1->ARRH = (uint8_t)(1000>>8);//�Զ�װ��ֵ
  TIM1->ARRL = (uint8_t)1000;
  TIM1->PSCRH = 0;//12��Ƶ
  TIM1->PSCRL = 12;
  TIM1->CR1 = TIM1_CR1_ARPE|TIM1_CR1_CEN;//���ϼ����Զ����¿�ʼ����
  TIM1->RCR = 0;	 
	TIM1->IER |= TIM1_IER_UIE;//�ж�����	
}

//�豸��ʼ��
void drv_init(void)
{
	//ʱ������
		
	//�ⲿ����
	CLK->CKDIVR=0;//����Ƶ
	CLK->ECKR = 0x01; //�����ⲿ���پ�������
	while((CLK->ECKR & 0x02) == 0x00); // �ȴ��ⲿ��������׼����
	CLK->SWCR = CLK->SWCR | 0x02; // ��ʱ��Դ�л����ⲿ�ĸ��پ�������
	CLK->SWR = 0xB4; // ѡ��оƬ�ⲿ�ĸ�������Ϊ��ʱ��
	while((CLK->SWCR & 0x08) == 0); // �ȴ��л��ɹ�
	CLK->SWCR = CLK->SWCR & 0xFD; // ����л���־

	//�ڲ�16M RCʱ��2��Ƶ����ʱ��Ϊ8M  CPUʱ�Ӳ���Ƶ����8M  ,����ʱ��Ϊ��ʱ��8M 
	//CLK->CKDIVR=0X08;//hsi=16/2=8MHz  CPU:����Ƶ
	
	//LED�ڳ�ʼ��
	GPIOD->DDR|=(1<<2)|(1<<3);
	GPIOD->ODR|=(1<<2)|(1<<3); //��ʼ�����
	GPIOD->CR1|=(1<<2)|(1<<3);//�������
	GPIOD->CR2|=(1<<2)|(1<<3);//10MHz
	
	GPIOD->DDR|=(1<<4);
	GPIOD->ODR&=~(1<<4); //��ʼ�����
	GPIOD->CR1|=(1<<4);//�������
	GPIOD->CR2|=(1<<4);//10MHz
	
	//����IO��ʼ��
	/*
	GPIOC->DDR&=~(1<<4);
	GPIOC->CR1|=(1<<4);//������
	GPIOC->CR2&=~(1<<4);*/
	
	Timer1Open();//���ʱ��1ms(��ʱ��1)��ʼ��

	ser_init();
	
	//����WDT��λ�ʱ�� 64k/256/256= 1.02��
#if WDT_ENABLE>0
	IWDG->KR=0xcc;//ʹ��
	IWDG->KR=0x55;//���ʼĴ���ʹ��
	IWDG->PR=6;//256��Ƶ
	IWDG->RLR=0XFF;//��װ����ֵ
	IWDG->KR=0XAA;//��λ���Ź�	
#endif
	
	//���ж�����
	enableInterrupts();
}

void drv_enter_critical(void)
{
	disableInterrupts();
}

void drv_exit_critical(void)
{
	enableInterrupts();
}

void drv_wdt_reset(void)
{
	IWDG->KR=0XAA;//��λ���Ź�
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
			;//GPIOD->ODR|=(1<<3);
		else;
	} 
	else
	{
		if(port==IO_Y1)
			;//GPIOD->ODR&=~(1<<3);
		else;		
	}
}

//��ȡ��������
uint8_t drv_input(uint8_t port)
{
	uint8_t ret=0;
	
		
	return ret;
}

/////////////////////////////////////////////////////////////////////////////


/****************************************************************************
//EEPROM��������
****************************************************************************/
void drv_eeprom_read(uint16_t addr,uint8_t *buf,uint8_t size)
{
	uint8_t i;
	uint8_t *p=(uint8_t *)(0X4000+addr);
	
	for(i=0;i<size;i++)
		buf[i]=*(p++);
}

void drv_eeprom_write(uint16_t addr,uint8_t *buf,uint8_t size)
{
	uint8_t i;
	uint8_t *p=(uint8_t *)(0X4000+addr);
	
	do
	{
		FLASH->DUKR = 0xae; // д���һ����Կ
		FLASH->DUKR = 0x56; // д��ڶ�����Կ
	} while((FLASH->IAPSR & 0x08) == 0); // ������δ�ɹ�������������

	
	for(i=0;i<size;i++)
	{
		*p=buf[i];
		while((FLASH->IAPSR & 0x04) == 0); // �ȴ�д�����ɹ�
		p++;
	}
}
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//UART1��ӡ�������
****************************************************************************/
#if 0
void UART1_Init(void)
{
	uint16_t uart_div=12000000/9600; //����Ƶ��/������
	uint8_t tmp;
	
	CLK->PCKENR1 |= (1<<2);//UARTʱ������
	
	tmp=uart_div;
	tmp&=0x0f;
	tmp|=(uart_div>>8)&0xf0;
	UART1->BRR1=uart_div>>4;
	UART1->BRR2=tmp;
	
	UART1->CR2=UART1_CR2_TEN|UART1_CR2_REN;//|UART1_CR2_RIEN;//���ͽ���ʹ�� �����ж�ʹ��
	UART1->CR1=0;//UARTʹ��
	
	GPIOD->DDR|=(1<<5);//�������
	GPIOD->CR1|=(1<<5);
	GPIOD->CR2|=(1<<5);
	GPIOD->CR1|=(1<<6);//��������
}

PUTCHAR_PROTOTYPE
{
    while (!(UART1->SR & UART1_SR_TXE));
    UART1->DR = c;    
}

GETCHAR_PROTOTYPE
{
     while(!(UART1->SR & UART1_SR_RXNE));
     return ((uint8_t)UART1->DR);
}

void UartSend(uint8_t *buf,uint8_t size)
{
	uint8_t i;
	for(i=0;i<size;i++)
	{
		drv_wdt_reset();
		while (!(UART1->SR & UART1_SR_TXE));
		UART1->DR = buf[i];  
	}
}
void UartPutc(uint8_t c)
{
		while (!(UART1->SR & UART1_SR_TXE));
		UART1->DR = c; 
}
#endif

/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//����(UART)����
****************************************************************************/
HQUEUE g_RxQueue; //�������ݶ���
HQUEUE g_TxQueue; //�������ݶ���
@tiny uint8_t g_RxBuffer[SERIAL_RECV_BUFFER_SIZE];	//���ջ���
@tiny uint8_t g_TxBuffer[SERIAL_SEND_BUFFER_SIZE];	//���ͻ���
uint8_t g_RecvWaitCounter;//����ȷ��ʱ�������

//��ʱ��4����Ϊ1ms��ʱ��
void ser_timer_init(void)
{
		//TIM4��ʱ��ʱ������
	  CLK->PCKENR1 |= (1<<4);
		
    TIM4->ARR = 93;//�Զ�װ��ֵ
		
		TIM4->PSCR=7;//��Ƶ128

    //���ϼ����Զ����¿�ʼ����
    TIM4->CR1 = TIM4_CR1_ARPE|TIM4_CR1_CEN; 
}

void ser_timer_enable(void)
{
	g_RecvWaitCounter=0;
	TIM4->SR1=0;								//���жϱ��
	TIM4->IER|=TIM4_IER_UIE; 		//�ж�����
}

void ser_timer_disable(void)
{
	TIM4->IER&=~TIM4_IER_UIE;//�жϽ�ֹ 
}

//��ʱ���ж� ����ʾ֡�������
INTERRUPT void TIM4_IRQHandler(void)
{
	if(++g_RecvWaitCounter >=SERIAL_RECV_AFFIRM_TIME)
	{
		ser_timer_disable();	
		frm_set_event(EVENT_UART_ID);
	}
	TIM4->SR1 =0;	
}

void ser_init(void)
{
	//uartӲ����ʼ�� 
	uint16_t uart_div=12000000/SERIAL_BAUD; //����Ƶ��/������
	uint8_t tmp;
	
	//CLK->PCKENR1 |= (1<<2);//UART1ʱ������
	
	tmp=uart_div;
	tmp&=0x0f;
	tmp|=(uart_div>>8)&0xf0;
	UART1->BRR1=uart_div>>4;
	UART1->BRR2=tmp;
	
	UART1->CR2=UART1_CR2_TEN|UART1_CR2_REN|UART1_CR2_RIEN;//���ͽ���ʹ�� �����ж�ʹ��
	UART1->CR1=0;//UARTʹ��
	
	//����/���ն��г�ʼ��
	QueueCreate(&g_RxQueue,g_RxBuffer,SERIAL_RECV_BUFFER_SIZE);
	QueueCreate(&g_TxQueue,g_TxBuffer,SERIAL_SEND_BUFFER_SIZE);
	
	//���ݰ��ж���ʱ����ʼ����ֹͣ
	ser_timer_init();
	ser_timer_disable();
}

INTERRUPT void UART1_RX_Handler(void)
{
	uint8_t recv=UART1->DR;
	if(frm_get_event_state(EVENT_UART_ID))//�ϴ����ݰ���δ����
		return ;
	QueueInputFromISR(&g_RxQueue,recv);
	ser_timer_enable();
}

//�����ж�
INTERRUPT void UART1_TX_Handler(void)
{
	if(UART1->SR & UART1_SR_TC)//�������
	{
		UART1->CR2&=~UART1_CR2_TCIEN;//��������жϽ���
	}
	else //���ͼĴ�����
	{
		if(QueueGetSize(&g_TxQueue)>0)
			UART1->DR=QueueOutputFromISR(&g_TxQueue);	
		else
		{
			UART1->CR2|=UART1_CR2_TCIEN; //��������ж�ʹ��
			UART1->CR2&=~UART1_CR2_TIEN; //���ͼĴ������жϽ�ֹ
		}
	}
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

	i=UART1->SR;//��һ��Ϊ�巢����ɱ��
	UART1->DR=buf[0];//���͵�һ���ֽ�
	UART1->CR2|=UART1_CR2_TIEN;//���ͼĴ������ж�����
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
//
****************************************************************************/

/////////////////////////////////////////////////////////////////////////////



