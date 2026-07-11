//serial.c - STM32F103C8 USART驱动 

#include "CAFrame.h" 

//-------------------------------------------------------------------------------USART1

#define UART1_BUF_SIZE SERIAL_BUFF_SIZE
static uint8_t g_Uart1RxBuf[UART1_BUF_SIZE];
static uint8_t g_Uart1TxBuf[UART1_BUF_SIZE];
static uint16_t g_Uart1TxIndex;
volatile uint16_t g_Uart1TxSize;
static uint16_t g_Uart1RxIndex;
volatile uint8_t g_Uart1RxFlag=0; //0表示未接收  1~5:正在接收数据包 >=6 已接收一数据包


//USART1初始化
void Uart1Init(uint32_t ulWantedBaud)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Peripheral clock enable
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  /**USART1 GPIO Configuration
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // USART1 interrupt Init
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),PRIORITY_DEF, PRIORITY_SUB));
  NVIC_EnableIRQ(USART1_IRQn);


  USART_InitStruct.BaudRate = ulWantedBaud;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);

	
	LL_USART_EnableIT_RXNE(USART1);//接收中断使能
	
  LL_USART_Enable(USART1);
}


//读接收缓冲数据
int Uart1Recv(uint8_t *buf)
{
	int i;
	
	if(g_Uart1RxFlag<6)
		return 0;
	
	CLI();
	for(i=0;i<g_Uart1RxIndex;i++)
		buf[i]=g_Uart1RxBuf[i];

	g_Uart1RxFlag=0;
	SEI();
	
	return i;
}

uint8_t drv_serial_input(uint8_t *buf)
{
	return (uint8_t)Uart1Recv(buf);
}

//UART1阻塞发送功能
uint8_t Uart1Send(uint8_t *buf,int len)
{
	uint32_t i,t;
	
	for(i=0;i<len;i++)
		g_Uart1TxBuf[i]=buf[i];
	
	g_Uart1TxIndex=0;
	g_Uart1TxSize=len;

	LL_USART_EnableIT_TXE(USART1);//发送空中断使能

	t=drv_get_current_tick();
	while(g_Uart1TxIndex < g_Uart1TxSize)
	{
		if(drv_get_past_tick(t)>1000)
			return 0;
	}

	return 1;
}

void drv_serial_output(uint8_t *buf,uint8_t size)
{
	Uart1Send(buf,size);
}

//UART1异步发送
void Uart1Post(uint8_t *buf,int len)
{
	uint32_t i;
	
	for(i=0;i<len;i++)
		g_Uart1TxBuf[i]=buf[i];
	
	g_Uart1TxIndex=0;
	g_Uart1TxSize=len;

	LL_USART_EnableIT_TXE(USART1);//发送空中断使能
}

//UART1中断函数
void USART1_IRQHandler( void )
{
	uint8_t cChar;

	if(LL_USART_IsActiveFlag_TXE(USART1) != RESET )
	{
		if(g_Uart1TxIndex<g_Uart1TxSize)
			LL_USART_TransmitData8(USART1,g_Uart1TxBuf[g_Uart1TxIndex++]);
		else
		{
			LL_USART_DisableIT_TXE(USART1);
			//LL_USART_EnableIT_TC(USART1);
		}			
	}

	if(LL_USART_IsActiveFlag_TC(USART1) != RESET )	
	{

	}	
	
	if(LL_USART_IsActiveFlag_RXNE(USART1) != RESET )
	{
		cChar = LL_USART_ReceiveData8(USART1);
		if(g_Uart1RxIndex<UART1_BUF_SIZE)
			g_Uart1RxBuf[g_Uart1RxIndex++]=cChar;
		g_Uart1RxFlag=1; //进入接收数据包状态
	}	
}

//-------------------------------------------------------------------------------USART2(485)
#define UART2_BUF_SIZE SERIAL_BUFF_SIZE

static uint8_t g_Uart2RxBuf[UART2_BUF_SIZE];
static uint8_t g_Uart2TxBuf[UART2_BUF_SIZE];
volatile uint16_t g_Uart2TxIndex;
volatile uint16_t g_Uart2TxSize;
volatile uint8_t g_Uart2TxFlag;
volatile uint8_t g_Uart2RxFlag=0; //0表示未接收  1~5:正在接收数据包 >=6 已接收一数据包
volatile uint16_t g_Uart2RxIndex;
#define UART2_RX_SEL LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);
#define UART2_TX_SEL LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4);

//USART2初始化
void Uart2Init(uint32_t ulWantedBaud)
{
	LL_USART_InitTypeDef USART_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Peripheral clock enable 
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  //DIR
	LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_4);
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
  /**USART2 GPIO Configuration
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // USART2 interrupt Init
  NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),PRIORITY_DEF, PRIORITY_SUB));
  NVIC_EnableIRQ(USART2_IRQn);


  USART_InitStruct.BaudRate = ulWantedBaud;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART2);
	
	LL_USART_EnableIT_RXNE(USART2);//接收中断使能
	
  LL_USART_Enable(USART2);
}


//读接收缓冲数据
int Uart2Recv(uint8_t *buf)
{
	int i;
	
	if(g_Uart2RxFlag<6)
		return 0;
	
	CLI();
	for(i=0;i<g_Uart2RxIndex;i++)
		buf[i]=g_Uart2RxBuf[i];
	g_Uart2RxIndex=0;
	g_Uart2RxFlag=0;
	SEI();
	return i;
}

//阻塞发送功能
uint8_t Uart2Send(uint8_t *buf,int len)
{
	uint32_t i,t;
	
	for(i=0;i<len;i++)
		 g_Uart2TxBuf[i]=buf[i];
	g_Uart2TxSize=len;
	g_Uart2TxIndex=0;
	UART2_TX_SEL;
	LL_USART_EnableIT_TXE(USART2);//发送空中断使能

	t=drv_get_current_tick();
	g_Uart2TxFlag=1;
	while(g_Uart2TxFlag)
	{
		if(drv_get_past_tick(t)>1000)
		{
			UART2_RX_SEL;
			return 0;
		}
	}
	UART2_RX_SEL;
	return 1;
}

//异步发送
void Uart2Post(uint8_t *buf,int len)
{
	uint32_t i;
	
	for(i=0;i<len;i++)
		 g_Uart2TxBuf[i]=buf[i];
	g_Uart2TxSize=len;
	g_Uart2TxIndex=0;
	UART2_TX_SEL;
	LL_USART_EnableIT_TXE(USART2);//发送空中断使能
}

//中断函数
void USART2_IRQHandler( void )
{
	uint8_t cChar;

	if(LL_USART_IsActiveFlag_TXE(USART2) != RESET )
	{
		if(g_Uart2TxIndex<g_Uart2TxSize)
			LL_USART_TransmitData8(USART2,g_Uart2TxBuf[g_Uart2TxIndex++]);
		else
		{
			LL_USART_DisableIT_TXE(USART2);
			LL_USART_EnableIT_TC(USART2);
		}			
	}

	if(LL_USART_IsActiveFlag_TC(USART2) != RESET )	
	{
		LL_USART_DisableIT_TC(USART2);
		UART2_RX_SEL;
		g_Uart2TxFlag=0;
	}	
	
	if(LL_USART_IsActiveFlag_RXNE(USART2) != RESET )
	{
		cChar = LL_USART_ReceiveData8(USART2);
		if(g_Uart2RxIndex < UART2_BUF_SIZE)
			g_Uart2RxBuf[g_Uart2RxIndex++]=cChar;
		g_Uart2RxFlag=1; //进入接收数据包状态
	}	
}
//-------------------------------------------------------------------------------USART3

#define UART3_BUF_SIZE 250

static uint8_t g_Uart3RxBuf[UART3_BUF_SIZE];
static uint8_t g_Uart3TxBuf[UART3_BUF_SIZE];
static uint8_t g_Uart3RxIndex;
static uint8_t g_Uart3TxIndex;
volatile uint8_t g_Uart3TxSize;
volatile uint8_t g_Uart3RxFlag=0; //0表示未接收  1~5:正在接收数据包 >=6 已接收一数据包

//USART2初始化
void Uart3Init(uint32_t ulWantedBaud)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
	
  /**USART3 GPIO Configuration
  PB10   ------> USART3_TX
  PB11   ------> USART3_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // USART3 interrupt Init
  NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),PRIORITY_DEF, PRIORITY_SUB));
  NVIC_EnableIRQ(USART3_IRQn);


  USART_InitStruct.BaudRate = ulWantedBaud;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART3, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART3);
	
	LL_USART_EnableIT_RXNE(USART3);//接收中断使能	
  LL_USART_Enable(USART3);
}


//读接收缓冲数据
int Uart3Recv(uint8_t *buf)
{
	int i;
	
	if(g_Uart3RxFlag==0)
		return 0;

	CLI();
	for(i=0;i<g_Uart3RxIndex;i++)
		buf[i]=g_Uart3RxBuf[i];
	g_Uart3RxIndex=0;
	g_Uart3RxFlag=0;
	SEI();
	
	return i;
}

//阻塞发送功能
uint8_t Uart3Send(uint8_t *buf,int len)
{
	uint32_t i,t;
	
	for(i=0;i<len;i++)
		g_Uart3TxBuf[i]=buf[i];

	g_Uart3TxIndex=0;
	g_Uart3TxSize=len;
	
	LL_USART_EnableIT_TXE(USART3);//发送空中断使能

	t=drv_get_current_tick();
	while(g_Uart3TxIndex < g_Uart3TxSize)
	{
		if(drv_get_past_tick(t)>1000)
			return 0;
	}

	return 1;
}

//异步发送
void Uart3Post(uint8_t *buf,int len)
{
	uint32_t i;
	
	for(i=0;i<len;i++)
		g_Uart3TxBuf[i]=buf[i];

	g_Uart3TxIndex=0;
	g_Uart3TxSize=len;
	
	LL_USART_EnableIT_TXE(USART3);//发送空中断使能
}

//中断函数
void USART3_IRQHandler( void )
{
	uint8_t cChar;

	if(LL_USART_IsActiveFlag_TXE(USART3) != RESET )
	{
		if(g_Uart3TxIndex < g_Uart3TxSize)
			LL_USART_TransmitData8(USART3,g_Uart3TxBuf[g_Uart3TxIndex++]);
		else
		{
			LL_USART_DisableIT_TXE(USART3);
			//LL_USART_EnableIT_TC(USART3);
		}			
	}

	if(LL_USART_IsActiveFlag_TC(USART3) != RESET )	
	{

	}	
	
	if(LL_USART_IsActiveFlag_RXNE(USART3) != RESET )
	{
		cChar = LL_USART_ReceiveData8(USART3);
		if(g_Uart3RxIndex < UART3_BUF_SIZE)
			g_Uart3RxBuf[g_Uart3RxIndex++]=cChar;
		g_Uart3RxFlag=1; //进入接收数据包状态
	}	
}


//---------------------------------------------------------------------------------------串口接收数据包定时器

void SerialTimerInit(void)
{
  //(注：总线分频器大于1时TIM3时钟为总线时钟的两倍，也就是72MHz,祥情参考STM32参考手册7.2节)

	LL_TIM_InitTypeDef TIM_InitStruct = {0};

  //Peripheral clock enable 
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  // TIM3 interrupt Init
	NVIC_SetPriority(TIM3_IRQn,NVIC_EncodePriority(NVIC_GetPriorityGrouping(),PRIORITY_DEF,PRIORITY_SUB));
  NVIC_EnableIRQ(TIM3_IRQn);

  TIM_InitStruct.Prescaler = 36;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 1000;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
  LL_TIM_EnableARRPreload(TIM3);
	LL_TIM_EnableCounter(TIM3);
	LL_TIM_EnableIT_UPDATE(TIM3);
}

//中断函数
void TIM3_IRQHandler(void)
{
	LL_TIM_ClearFlag_UPDATE(TIM3);
	
	if(g_Uart1RxFlag>0)
	{
		if(g_Uart1RxFlag < 7) 
			g_Uart1RxFlag++;
		if(g_Uart1RxFlag == 6)
		{
			frm_set_event(EVENT_UART_ID);
		}
	}
	
	if(g_Uart2RxFlag > 0)
	{
		if(g_Uart2RxFlag < 7) 
			g_Uart2RxFlag++;
		if(g_Uart2RxFlag ==6)
		{
			frm_set_event(EVENT_UART2_ID);
		}
	}
	
	if(g_Uart3RxFlag > 0 )
	{
		if(g_Uart3RxFlag < 7) 
			g_Uart3RxFlag++;
		if(g_Uart3RxFlag==6)
			frm_set_event(EVENT_UART3_ID);
	}
}

