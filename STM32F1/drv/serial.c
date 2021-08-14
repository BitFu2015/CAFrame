//STM32F103C8T6�������� serial.c

#include "caframe.h"

/////////////////////////////////////////////////////////////////////////////////////////////UART1
//-------------------------------------------------------------------------------------------RS485��
//UART1 ����
#define UART1_BAUD 9600
#define UART1_BUF_SIZE 128
static HQUEUE xRxQueue1;
static HQUEUE xTxQueue1;
static uint8_t g_Uart1RxBuf[UART1_BUF_SIZE];
static uint8_t g_Uart1TxBuf[UART1_BUF_SIZE];
volatile uint8_t g_Uart1SendFlag;
volatile uint8_t g_Uart1RecvFlag=0; //0��ʾδ����  1~5:���ڽ������ݰ� >=6 �ѽ���һ���ݰ�
#define UART1_TX_SEL  GPIO_SetBits(GPIOB , GPIO_Pin_5)
#define UART1_RX_SEL  GPIO_ResetBits(GPIOB,GPIO_Pin_5)

//UART1��ʼ��
void Uart1Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
		
	QueueCreate(&xRxQueue1,g_Uart1RxBuf,UART1_BUF_SIZE);
	QueueCreate(&xTxQueue1,g_Uart1TxBuf,UART1_BUF_SIZE);

	// Enable USART1 clock
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );	
	
	//��������ӳ��
	GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);

	// Configure USART1 Rx (PB7) as input floating
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	
	// Configure USART1 Tx (PB6) as alternate function push-pull 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	
	//485����˿�
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	UART1_RX_SEL;
	
	USART_InitStructure.USART_BaudRate = UART1_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( USART1, &USART_InitStructure );
	
	USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );  	//�����ж�ʹ��
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DEFAULT_INT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );
	
	USART_Cmd( USART1, ENABLE );
}

//UART1��ȡ�����ַ�
/*
uint8_t Uart1ReadByte(uint8_t *pcRxedChar,uint32_t dly)
{
	uint32_t tck=GetCurrentTick(); //��¼��ʼʱ������

	while(QueueGetSize(&xRxQueue1)==0)//������������
	{
		if(GetPassedTick(tck)>=dly)	  //��ʱ
			return 0;
	}

	*pcRxedChar=QueueOutput(&xRxQueue1);

	return 1;
}*/

//�����ջ�������
uint8_t Uart1ReadRecv(uint8_t *buf)
{
	uint8_t i;
	
	if(g_Uart1RecvFlag<6)
		return 0;
	i=0;
	while(QueueGetSize(&xRxQueue1))
		buf[i++]=QueueOutput(&xRxQueue1);
	g_Uart1RecvFlag=0;
	return i;
}

//UART1���͹���
void Uart1Send(uint8_t *buf,int len)
{
		uint32_t i;
	
	for(i=0;i<len;i++)
		 QueueInput( &xTxQueue1, buf[i]);	

	UART1_TX_SEL;
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE );	//�ж�ʹ��
	g_Uart1SendFlag=0;
	i=GetCurrentTick();
	while(g_Uart1SendFlag==0)
	{
		if(GetPassedTick(i)>1000)
		{
			Uart1Init();
			break;
		}	
	}
	UART1_RX_SEL;
}

void Uart1Post(uint8_t *buf,int len)
{
	uint32_t i;

	for(i=0;i<len;i++)
		QueueInput( &xTxQueue1, buf[i]);	

	UART1_TX_SEL;
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE );	//�ж�ʹ��	
}

//UART1�жϺ���
void USART1_IRQHandler( void )
{
	uint8_t cChar;

	if(USART_GetITStatus( USART1, USART_IT_TXE ) == SET )
	{
		if(QueueGetSize(&xTxQueue1))
			USART_SendData(USART1,QueueOutputFromISR(&xTxQueue1));
		else
		{
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE ); //�رշ����ж�		
			USART_ITConfig(USART1, USART_IT_TC,ENABLE);	//�򿪷�������ж�
		}			
	}

	if(USART_GetITStatus( USART1, USART_IT_TC ) == SET)
	{
		g_Uart1SendFlag=1;
		USART_ITConfig(USART1, USART_IT_TC,DISABLE);
		UART1_RX_SEL;
	}	
	
	if(USART_GetITStatus( USART1, USART_IT_RXNE ) == SET )
	{
		cChar = USART_ReceiveData( USART1 );
		QueueInputFromISR( &xRxQueue1, cChar);
		g_Uart1RecvFlag=1; //����������ݰ�״̬
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////UART2
//-------------------------------------------------------------------------------------------RS485-2��
//UART2 ����
#define UART2_BAUD 115200
#define UART2_BUF_SIZE 128
static HQUEUE xRxQueue2; //������ȡ���ݰ�����
static HQUEUE xTxQueue2;
static uint8_t g_Uart2RxBuf[UART2_BUF_SIZE];
static uint8_t g_Uart2TxBuf[UART2_BUF_SIZE];
volatile uint8_t g_Uart2SendFlag;
volatile uint8_t g_Uart2RecvFlag=0; 
#define UART2_TX_SEL  GPIO_SetBits(GPIOB , GPIO_Pin_0)
#define UART2_RX_SEL  GPIO_ResetBits(GPIOB,GPIO_Pin_0)

//UART2��ʼ��
void Uart2Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	QueueCreate(&xRxQueue2,g_Uart2RxBuf,UART2_BUF_SIZE-2);
	QueueCreate(&xTxQueue2,g_Uart2TxBuf,UART2_BUF_SIZE-2);

	//����ʱ�Ӻ�IO���ų�ʼ��
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );	

	//Configure USART2 Rx (PA3) as input floating 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	
	//Configure USART2 Tx (PA2) as alternate function push-pull
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init( GPIOA, &GPIO_InitStructure );
	
	//485����˿�
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	UART2_RX_SEL;

	//�ж�����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;

	USART_InitStructure.USART_BaudRate = UART2_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( USART2, &USART_InitStructure );
	
	USART_ITConfig( USART2, USART_IT_RXNE, ENABLE );  	//�����ж�ʹ��
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =DEFAULT_INT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );
	
	USART_Cmd( USART2, ENABLE );
}

//UART2
/*
uint8_t Uart2ReadByte(uint8_t *pcRxedChar,uint32_t dly)
{
	uint32_t tck=GetCurrentTick();

	while(QueueGetSize(&xRxQueue2)==0)
	{
		if(GetPassedTick(tck)>=dly)
			return 0;
	}

	*pcRxedChar=QueueOutput(&xRxQueue2);

	return 1;
}*/

//�����ջ�������
uint8_t Uart2ReadRecv(uint8_t *buf)
{
	uint8_t i;
	
	if(g_Uart2RecvFlag<6)
		return 0;
	i=0;
	while(QueueGetSize(&xRxQueue2))
		buf[i++]=QueueOutput(&xRxQueue2);
	g_Uart2RecvFlag=0;
	return i;
}

//UART2�������ݲ��ȴ����������
void Uart2Send(uint8_t *buf,int len)
{
	uint32_t i;
	
	for(i=0;i<len;i++)
		 QueueInput( &xTxQueue2, buf[i]);	

	USART_ITConfig(USART2, USART_IT_TXE, ENABLE );	//�ж�ʹ��
	g_Uart2SendFlag=0;
	i=GetCurrentTick();
	while(g_Uart2SendFlag==0)
	{
		if(GetPassedTick(i)>1000)
		{
			Uart2Init();
			break;
		}	
	}
}

//UART2���͹��ܺ���,��������
void Uart2Post(uint8_t *buf,int len)
{
	int i;

	for(i=0;i<len;i++)
		 QueueInput(&xTxQueue2, buf[i]);
	
	USART_ITConfig( USART2, USART_IT_TXE, ENABLE );	//�ж�ʹ��
}

//UART2�жϺ���
void USART2_IRQHandler( void )
{
	uint8_t cChar;

	if( USART_GetITStatus( USART2, USART_IT_TXE ) == SET )
	{
		if(QueueGetSize(&xTxQueue2) )
			USART_SendData( USART2,QueueOutputFromISR(&xTxQueue2));
		else
		{
			USART_ITConfig( USART2, USART_IT_TXE, DISABLE );
			g_Uart2SendFlag=1;
		}
	}
	
	if( USART_GetITStatus( USART2, USART_IT_RXNE) == SET )
	{
		cChar = USART_ReceiveData( USART2 );
		QueueInputFromISR(&xRxQueue2, cChar);	
		g_Uart2RecvFlag=1; //����������ݰ�״̬
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////UART3
//-------------------------------------------------------------------------------------------RS485-3��
//UART3 ����
#define UART3_BAUD 115200
#define UART3_BUF_SIZE 128							
static HQUEUE xRxQueue3;
static HQUEUE xTxQueue3;
static uint8_t g_Uart3RxBuf[UART3_BUF_SIZE];
static uint8_t g_Uart3TxBuf[UART3_BUF_SIZE];
volatile uint8_t g_Uart3SendFlag; //
volatile uint8_t g_Uart3RecvFlag=0; 
#define UART3_TX_SEL  GPIO_SetBits(GPIOB , GPIO_Pin_2)
#define UART3_RX_SEL  GPIO_ResetBits(GPIOB,GPIO_Pin_2)

//UART3��ʼ��
void Uart3Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	QueueCreate(&xRxQueue3,g_Uart3RxBuf,UART3_BUF_SIZE);
	QueueCreate(&xTxQueue3,g_Uart3TxBuf,UART3_BUF_SIZE);

	//Enable clock 
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART3, ENABLE );	

	//Configure USART3 Rx (PB11) as input floating 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	
	//Configure USART3 Tx (PB10) as alternate function push-pull
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init( GPIOB, &GPIO_InitStructure );
	
	//485����˿�
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	UART3_RX_SEL;

	//�ж�����
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;

	USART_InitStructure.USART_BaudRate = UART3_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init( USART3, &USART_InitStructure );
	
	USART_ITConfig( USART3, USART_IT_RXNE, ENABLE );	//�����ж�ʹ��
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DEFAULT_INT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );
	
	USART_Cmd( USART3, ENABLE );
}

//UART3
/*
uint8_t Uart3ReadByte(uint8_t *pcRxedChar,uint32_t dly)
{
	uint32_t tck=GetCurrentTick();

	while(QueueGetSize(&xRxQueue3)==0)
	{
		if(GetPassedTick(tck)>=dly)
			return 0;
	}

	*pcRxedChar=QueueOutput(&xRxQueue3);

	return 1;
}*/
//�����ջ�������
uint8_t Uart3ReadRecv(uint8_t *buf)
{
	uint8_t i;
	
	if(g_Uart3RecvFlag<6)
		return 0;
	i=0;
	while(QueueGetSize(&xRxQueue3))
		buf[i++]=QueueOutput(&xRxQueue3);
	g_Uart3RecvFlag=0;
	return i;
}
//UART3���͹���
void Uart3Send(uint8_t *buf,int len)
{
	uint32_t i;
	
	for(i=0;i<len;i++)
		 QueueInput( &xTxQueue3, buf[i]);	

	USART_ITConfig(USART3, USART_IT_TXE, ENABLE );	//�ж�ʹ��
	g_Uart3SendFlag=0;
	i=GetCurrentTick();
	while(g_Uart3SendFlag==0)
	{
		if(GetPassedTick(i)>1000)
		{
			Uart3Init();
			break;
		}	
	}
}

void Uart3Post(uint8_t *buf,int len)
{
	uint32_t i;
	
	for(i=0;i<len;i++)
		 QueueInput( &xTxQueue3, buf[i]);	
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE );	//�ж�ʹ��
}

//UART3�жϺ���
void USART3_IRQHandler(void)
{
	uint8_t cChar;

	if(USART_GetITStatus( USART3, USART_IT_TXE ) == SET)
	{
		if(QueueGetSize(&xTxQueue3))
			USART_SendData( USART3, QueueOutputFromISR(&xTxQueue3));
		else
		{
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
			g_Uart3SendFlag=1;
		}			
	}
	
	if( USART_GetITStatus( USART3, USART_IT_RXNE ) == SET )
	{
		cChar = USART_ReceiveData( USART3 );
		QueueInputFromISR( &xRxQueue3, cChar);
		g_Uart3RecvFlag=1; //����������ݰ�״̬
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////�������ݰ��ж���ʱ�� TIM3
//1ms��ʱ��ʱ��
void UartTimerInit(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef         NVIC_InitStructure;

	//tim3
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	//��ʼ�� (ע��ʱ��Ϊ����ʱ�ӵ�������Ҳ����72MHz,����ο�STM32�ο��ֲ�7.2��)
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseStructure.TIM_Period =10;			//����ֵ
	TIM_TimeBaseStructure.TIM_Prescaler = 7200;		//Ԥ��Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1  ;	//�˲�����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DEFAULT_INT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//�ж�����
	TIM_Cmd(TIM3, ENABLE);	
}

void TIM3_IRQHandler(void)
{	
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);	

	if((g_Uart1RecvFlag > 0 )&&(g_Uart1RecvFlag<6))
	{
		if(++g_Uart1RecvFlag >=6)
			frm_set_event(EVENT_UART1_ID);
	}
	

	if((g_Uart2RecvFlag > 0 )&&(g_Uart2RecvFlag<6))
	{
		if(++g_Uart2RecvFlag>=6)
			frm_set_event(EVENT_UART2_ID);
	}

	if((g_Uart3RecvFlag > 0 )&&(g_Uart3RecvFlag<6))
	{
			if(++g_Uart3RecvFlag >= 6)
				frm_set_event(EVENT_UART3_ID);
	}		
}

