//STM32F103C8T6串口驱动 serial.c

#include "caframe.h"

/////////////////////////////////////////////////////////////////////////////////////////////UART1
//-------------------------------------------------------------------------------------------RS485口
//UART1 队列
#define UART1_BAUD 9600
#define UART1_BUF_SIZE 128
static HQUEUE xRxQueue1;
static HQUEUE xTxQueue1;
static uint8_t g_Uart1RxBuf[UART1_BUF_SIZE];
static uint8_t g_Uart1TxBuf[UART1_BUF_SIZE];
volatile uint8_t g_Uart1SendFlag;
volatile uint8_t g_Uart1RecvFlag=0; //0表示未接收  1~5:正在接收数据包 >=6 已接收一数据包
#define UART1_TX_SEL  GPIO_SetBits(GPIOB , GPIO_Pin_5)
#define UART1_RX_SEL  GPIO_ResetBits(GPIOB,GPIO_Pin_5)

//UART1初始化
void Uart1Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
		
	QueueCreate(&xRxQueue1,g_Uart1RxBuf,UART1_BUF_SIZE);
	QueueCreate(&xTxQueue1,g_Uart1TxBuf,UART1_BUF_SIZE);

	// Enable USART1 clock
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );	
	
	//引脚重新映射
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
	
	//485方向端口
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
	
	USART_ITConfig( USART1, USART_IT_RXNE, ENABLE );  	//接收中断使能
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DEFAULT_INT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init( &NVIC_InitStructure );
	
	USART_Cmd( USART1, ENABLE );
}

//UART1读取接收字符
/*
uint8_t Uart1ReadByte(uint8_t *pcRxedChar,uint32_t dly)
{
	uint32_t tck=GetCurrentTick(); //记录开始时间种子

	while(QueueGetSize(&xRxQueue1)==0)//缓冲区无数据
	{
		if(GetPassedTick(tck)>=dly)	  //超时
			return 0;
	}

	*pcRxedChar=QueueOutput(&xRxQueue1);

	return 1;
}*/

//读接收缓冲数据
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

//UART1发送功能
void Uart1Send(uint8_t *buf,int len)
{
		uint32_t i;
	
	for(i=0;i<len;i++)
		 QueueInput( &xTxQueue1, buf[i]);	

	UART1_TX_SEL;
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE );	//中断使能
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
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE );	//中断使能	
}

//UART1中断函数
void USART1_IRQHandler( void )
{
	uint8_t cChar;

	if(USART_GetITStatus( USART1, USART_IT_TXE ) == SET )
	{
		if(QueueGetSize(&xTxQueue1))
			USART_SendData(USART1,QueueOutputFromISR(&xTxQueue1));
		else
		{
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE ); //关闭发送中断		
			USART_ITConfig(USART1, USART_IT_TC,ENABLE);	//打开发送完成中断
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
		g_Uart1RecvFlag=1; //进入接收数据包状态
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////UART2
//-------------------------------------------------------------------------------------------RS485-2口
//UART2 队列
#define UART2_BAUD 115200
#define UART2_BUF_SIZE 128
static HQUEUE xRxQueue2; //变量读取数据包接收
static HQUEUE xTxQueue2;
static uint8_t g_Uart2RxBuf[UART2_BUF_SIZE];
static uint8_t g_Uart2TxBuf[UART2_BUF_SIZE];
volatile uint8_t g_Uart2SendFlag;
volatile uint8_t g_Uart2RecvFlag=0; 
#define UART2_TX_SEL  GPIO_SetBits(GPIOB , GPIO_Pin_0)
#define UART2_RX_SEL  GPIO_ResetBits(GPIOB,GPIO_Pin_0)

//UART2初始化
void Uart2Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	QueueCreate(&xRxQueue2,g_Uart2RxBuf,UART2_BUF_SIZE-2);
	QueueCreate(&xTxQueue2,g_Uart2TxBuf,UART2_BUF_SIZE-2);

	//外设时钟和IO引脚初始化
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
	
	//485方向端口
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	UART2_RX_SEL;

	//中断设置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;

	USART_InitStructure.USART_BaudRate = UART2_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init( USART2, &USART_InitStructure );
	
	USART_ITConfig( USART2, USART_IT_RXNE, ENABLE );  	//接收中断使能
	
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

//读接收缓冲数据
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

//UART2发送数据并等待到发送完成
void Uart2Send(uint8_t *buf,int len)
{
	uint32_t i;
	
	for(i=0;i<len;i++)
		 QueueInput( &xTxQueue2, buf[i]);	

	USART_ITConfig(USART2, USART_IT_TXE, ENABLE );	//中断使能
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

//UART2发送功能函数,立即返回
void Uart2Post(uint8_t *buf,int len)
{
	int i;

	for(i=0;i<len;i++)
		 QueueInput(&xTxQueue2, buf[i]);
	
	USART_ITConfig( USART2, USART_IT_TXE, ENABLE );	//中断使能
}

//UART2中断函数
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
		g_Uart2RecvFlag=1; //进入接收数据包状态
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////UART3
//-------------------------------------------------------------------------------------------RS485-3口
//UART3 队列
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

//UART3初始化
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
	
	//485方向端口
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	UART3_RX_SEL;

	//中断设置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;

	USART_InitStructure.USART_BaudRate = UART3_BAUD;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init( USART3, &USART_InitStructure );
	
	USART_ITConfig( USART3, USART_IT_RXNE, ENABLE );	//接收中断使能
	
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
//读接收缓冲数据
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
//UART3发送功能
void Uart3Send(uint8_t *buf,int len)
{
	uint32_t i;
	
	for(i=0;i<len;i++)
		 QueueInput( &xTxQueue3, buf[i]);	

	USART_ITConfig(USART3, USART_IT_TXE, ENABLE );	//中断使能
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
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE );	//中断使能
}

//UART3中断函数
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
		g_Uart3RecvFlag=1; //进入接收数据包状态
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////串口数据包判定定时器 TIM3
//1ms超时定时器
void UartTimerInit(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef         NVIC_InitStructure;

	//tim3
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	//初始化 (注：时钟为总线时钟的两倍，也就是72MHz,祥情参考STM32参考手册7.2节)
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseStructure.TIM_Period =10;			//上限值
	TIM_TimeBaseStructure.TIM_Prescaler = 7200;		//预分频
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1  ;	//滤波器分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = DEFAULT_INT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);//中断允许
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

