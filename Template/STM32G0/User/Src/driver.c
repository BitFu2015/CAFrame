//driver.c - CAFrame工程模板 驱动程序STM32G0单片机示例 

#include "CAFrame.h"

//**-----------------------------------------------------------------------------------------------时钟/系统定时器
volatile static uint32_t g_SysTick=0;
void SerialTimerTick(void);
void USART1_Init(uint32_t baud);
void USART2_Init(uint32_t baud);

/****************************************************************************
//框架通用接口函数(框架要求在驱动中实现)
//注：函数原型已在CAFrame.h中声明
****************************************************************************/
//系统定时器中断 1ms中断
void SysTick_Handler(void)
{
	g_SysTick++;
	SerialTimerTick();
  frm_inc_timer_tick();
}

static void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
  {
  }

  //HSI configuration and activation 
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  //Main PLL configuration and activation 
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_Enable();
  LL_RCC_PLL_EnableDomain_SYS();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  // Set AHB prescaler
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  // Sysclk activation on the main PLL 
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  // Set APB1 prescaler
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	//LL_Init1msTick(64000000);
	//NVIC_SetPriority(SysTick_IRQn, 0x0);
	SysTick_Config(64000);

  // Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) 
  LL_SetSystemCoreClock(64000000);
}


//设备初始化
void drv_init(void)
{	
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};	
	
	SystemClock_Config();
	
  // GPIO Ports Clock Enable 
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

  //LED LEDR LEDG LEDB
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4|LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  //KEY
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  //Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  LL_SYSCFG_EnablePinRemap(LL_SYSCFG_PIN_RMP_PA11);
  LL_SYSCFG_EnablePinRemap(LL_SYSCFG_PIN_RMP_PA12);

	USART1_Init(9600);
	USART2_Init(9600);
	LED_ON;

	//iwdg
	LL_IWDG_Enable(IWDG);
	LL_IWDG_EnableWriteAccess(IWDG);
	LL_IWDG_SetPrescaler(IWDG,LL_IWDG_PRESCALER_64);//时钟为内部40KHz 分频64
	LL_IWDG_SetReloadCounter(IWDG,1000);
	while(LL_IWDG_IsReady(IWDG) == 0);
	LL_IWDG_ReloadCounter(IWDG);//复位
}

void drv_enter_critical(void)
{
	CLI();	//__disable_irq();
}

void drv_exit_critical(void)
{
	SEI();	//__enable_irq() ;
}

void drv_wdt_reset(void)
{
	LL_IWDG_ReloadCounter(IWDG);
}

void drv_sys_reset(void)
{
	__disable_irq();
//	__set_PRIMASK(1);
	//__set_FAULTMASK(1);
	//__set_FAULTMASK(1);//关闭所有中断
	NVIC_SystemReset();//复位函数
}

//输出控制接口函数
void drv_output(uint8_t port,uint8_t val)
{
	if(val)
	{
		if(port==IO_Y1)
			LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4);
		else;
	}
	else
	{
		if(port==IO_Y1)
			LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);
		else;
	}
}

//获取输入检测结果
uint8_t drv_input(uint8_t port)
{
	uint8_t ret=0;

	if(port==IO_X1)
	{
			ret=LL_GPIO_IsInputPinSet(GPIOA,LL_GPIO_PIN_5);
	}
		
	return ret;
}

/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//框架标准化接口函数(框架本身不调用)
//注：函数原型已在CAFrame.h中声明
****************************************************************************/
UINT drv_get_current_tick(void)
{
	return g_SysTick;
}

UINT drv_get_past_tick(uint32_t t)
{
	uint32_t tmp=g_SysTick;
	return tmp-t;
}

//注：由于虚拟EEPOM,所以无法按地址存取,只能读取当前一块数据
void drv_eeprom_read(uint16_t addr,uint8_t *buf,uint8_t size)
{
	EepRead(buf,size);
}

//注：由于虚拟EEPOM,所以无法按地址存取，只能写入当前一块数据
void drv_eeprom_write(uint16_t addr,uint8_t *buf,uint8_t size)
{
	EepWrite(buf,size);
}

/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//用户扩展驱动
//注：用户扩展驱动接口函数原型需要在driver.h中声明，以便在APP中调用
****************************************************************************/
//**-----------------------------------------------------------------------------------------------延时
void drv_delay_us(uint32_t t)
{
	uint32_t i;
	volatile uint32_t j;
	for(i=0;i<t;i++)
	{
		j=6;
		while(j--)
		__asm("nop");
	}
}

void drv_delay_ms(uint32_t t)
{
	uint32_t i;
	for(i=0;i<t;i++)
	{
		drv_delay_us(1000);
		drv_wdt_reset();
	}
}

//**-----------------------------------------------------------------------------------------------USART1
#define USART1_BUF_SIZE SERIAL_BUFF_SIZE
static uint8_t g_Rx1Buffer[USART1_BUF_SIZE];//接收缓冲区
static uint8_t g_Tx1Buffer[USART1_BUF_SIZE];//发送缓冲区
 uint32_t g_Rx1Index;//接收字节索引
static volatile uint8_t g_Rx1Flag;//接收数据包标志
static uint8_t g_Rx1Timer;//数据包接收完成计数器
static volatile uint8_t g_Tx1Flag;//发送数据包标志
static uint32_t g_Tx1Size;	//发送长度
static uint32_t g_Tx1Index;	//发送索引

void USART1_Init(uint32_t baud)
{

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);

  //Peripheral clock enable
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
	
	LL_SYSCFG_EnablePinRemap(LL_SYSCFG_PIN_RMP_PA11);
	LL_SYSCFG_EnablePinRemap(LL_SYSCFG_PIN_RMP_PA12);

  //PA9   ------> USART1_TX
	//PA10   ------> USART1_RX
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = baud;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
	
  LL_USART_SetTXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_SetRXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_DisableFIFO(USART1);
  LL_USART_ConfigAsyncMode(USART1);
	
	//中断打开
	NVIC_SetPriority(USART1_IRQn, 1);
	NVIC_EnableIRQ(USART1_IRQn);
	
	LL_USART_EnableIT_RXNE_RXFNE(USART1);//接收中断打开	
	
//	LL_USART_SetRxTimeout(USART2,39);//接收时字节间隔为3.5个字节时间时认定数据包结束
//	LL_USART_EnableRxTimeout(USART2);

  LL_USART_Enable(USART1);
	
	//状态量初始化
	g_Tx1Index=0;
	g_Tx1Size=0;
	g_Tx1Flag=0;
	
	g_Rx1Index=0;	
	g_Rx1Flag=UART_RX_IDLE;
	g_Rx1Timer=0;

  //Polling USART1 initialisation
  while((!(LL_USART_IsActiveFlag_TEACK(USART1))) || (!(LL_USART_IsActiveFlag_REACK(USART1))))
  {
	}
}

//发送
uint32_t drv_serial_output(uint8_t *buf,uint32_t len)
{
	uint8_t i;
	
	if(g_Tx1Flag) //有未发送完成数据包
		return 0;
	
	for(i=0;i<len;i++)
		g_Tx1Buffer[i]=buf[i];
	
	g_Tx1Size=len;
	g_Tx1Index=0;
	g_Tx1Flag=1;//正在发送	
	
	LL_USART_ClearFlag_TC(USART1);
	LL_USART_EnableIT_TXE(USART1);//发送寄存器空中断允许
	return i;	
}

//读接收包
uint32_t drv_serial_input(uint8_t *buf)
{
	uint32_t i,len;
	
	if(g_Rx1Flag != UART_RX_READY)
		return 0;
	
	len=g_Rx1Index;
	for(i=0;i<len;i++)
		buf[i]=g_Rx1Buffer[i];
	
	do{g_Rx1Index=0;}while(g_Rx1Index);
	do{g_Rx1Flag=UART_RX_IDLE;}while(g_Rx1Flag!=UART_RX_IDLE);
	
	return len;	
}


//中断处理
void USART1_IRQHandler(void)
{
	uint8_t c;
	
	//接收到一个字节
	if(LL_USART_IsActiveFlag_RXNE_RXFNE(USART1))
	{
		c=LL_USART_ReceiveData8(USART1);
		if(g_Rx1Flag != UART_RX_READY)//没有已接收数据包
		{
			g_Rx1Timer=0;
			g_Rx1Flag=UART_RX_BUSY;
			if(g_Rx1Index<USART1_BUF_SIZE)
				g_Rx1Buffer[g_Rx1Index++]=c;
		}
		else
			frm_set_event(EVENT_UART_ID);
	}

	//发送完成处理
	if(LL_USART_IsActiveFlag_TC(USART1))
	{
		LL_USART_DisableIT_TC(USART1);//关闭发送完成中断
		LL_USART_ClearFlag_TC(USART1);
		//LL_USART_RequestTxDataFlush(USART2);
		g_Tx1Flag=0;
	}	
	else
	{
		//发送处理
		if(LL_USART_IsActiveFlag_TXE_TXFNF(USART1))
		{
			if(g_Tx1Size > g_Tx1Index)
				LL_USART_TransmitData8(USART1, g_Tx1Buffer[g_Tx1Index++]);
			else
			{
				LL_USART_EnableIT_TC(USART1);//发送完成中断使能
				LL_USART_DisableIT_TXE(USART1);//发送寄存器空中断禁止
			}
		}				
	}
}

//**-----------------------------------------------------------------------------------------------USART2
#define USART2_BUF_SIZE SERIAL_BUFF_SIZE
static uint8_t g_Rx2Buffer[USART2_BUF_SIZE];//接收缓冲区
static uint8_t g_Tx2Buffer[USART2_BUF_SIZE];//发送缓冲区
uint32_t g_Rx2Index;//接收字节索引
static volatile uint8_t g_Rx2Flag;//接收数据包标志
static uint8_t g_Rx2Timer;//数据包接收完成计数器
static volatile uint8_t g_Tx2Flag;//发送数据包标志
static uint32_t g_Tx2Size;	//发送长度
static uint32_t g_Tx2Index;	//发送索引

void USART2_Init(uint32_t baud)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	LL_USART_DeInit(USART2);
	
  // Peripheral clock enable
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = baud;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
	
	LL_USART_SetTXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_SetRXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_DisableFIFO(USART2);
  LL_USART_ConfigAsyncMode(USART2);
	
	//中断打开
	NVIC_SetPriority(USART2_IRQn, 1);
	NVIC_EnableIRQ(USART2_IRQn);
	
	LL_USART_EnableIT_RXNE_RXFNE(USART2);//接收中断打开	
	
//	LL_USART_SetRxTimeout(USART2,39);//接收时字节间隔为3.5个字节时间时认定数据包结束
//	LL_USART_EnableRxTimeout(USART2);

  LL_USART_Enable(USART2);
	
	//状态量初始化
	g_Tx2Index=0;
	g_Tx2Size=0;
	g_Tx2Flag=0;
	
	g_Rx2Index=0;	
	g_Rx2Flag=UART_RX_IDLE;
	g_Rx2Timer=0;

  //Polling USART2 initialisation
  while((!(LL_USART_IsActiveFlag_TEACK(USART2))) || (!(LL_USART_IsActiveFlag_REACK(USART2))))
  {
  }
}

//发送
uint32_t Serial2Write(uint8_t *buf,uint32_t len)
{
	uint8_t i;
	
	if(g_Tx2Flag) //有未发送完成数据包
		return 0;
	
	for(i=0;i<len;i++)
		g_Tx2Buffer[i]=buf[i];
	
	g_Tx2Size=len;
	g_Tx2Index=0;
	g_Tx2Flag=1;//正在发送	
	
	LL_USART_ClearFlag_TC(USART2);
	LL_USART_EnableIT_TXE(USART2);//发送寄存器空中断允许
	return i;	
}

//读接收包
uint32_t Serial2Read(uint8_t *buf)
{
	uint32_t i,len;
	
	if(g_Rx2Flag != UART_RX_READY)//无接收包
		return 0;
	
	len=g_Rx2Index;
	for(i=0;i<len;i++)
		buf[i]=g_Rx2Buffer[i];
	
	do{g_Rx2Index=0;}while(g_Rx2Index);
	do{g_Rx2Flag=UART_RX_IDLE;}while(g_Rx2Flag!=UART_RX_IDLE);//空闲状态
	
	return len;	
}


//中断处理
void USART2_IRQHandler(void)
{
	uint8_t c;
	
	//接收到一个字节
	if(LL_USART_IsActiveFlag_RXNE_RXFNE(USART2))
	{
		c=LL_USART_ReceiveData8(USART2);
		if(g_Rx2Flag != UART_RX_READY)//没有已接收数据包
		{
			g_Rx2Timer=0;
			g_Rx2Flag=UART_RX_BUSY;
			if(g_Rx2Index<USART2_BUF_SIZE)
				g_Rx2Buffer[g_Rx2Index++]=c;
		}
		else
			frm_set_event(EVENT_UART2_ID);
	}

	//发送完成处理
	if(LL_USART_IsActiveFlag_TC(USART2))
	{
		LL_USART_DisableIT_TC(USART2);//关闭发送完成中断
		LL_USART_ClearFlag_TC(USART2);
		//LL_USART_RequestTxDataFlush(USART2);
		g_Tx2Flag=0;
	}	
	else
	{
		//发送处理
		if(LL_USART_IsActiveFlag_TXE_TXFNF(USART2))
		{
			if(g_Tx2Size > g_Tx2Index)
				LL_USART_TransmitData8(USART2, g_Tx2Buffer[g_Tx2Index++]);
			else
			{
				LL_USART_EnableIT_TC(USART2);//发送完成中断使能
				LL_USART_DisableIT_TXE(USART2);//发送寄存器空中断禁止
			}
		}				
	}
}

//**-----------------------------------------------------------------------------------------------uart接收定时处理

void SerialTimerTick(void)
{
	if(g_Rx2Flag==UART_RX_BUSY)
	{
		if(++g_Rx2Timer>5)
		{
			g_Rx2Flag=UART_RX_READY;
			frm_set_event(EVENT_UART2_ID);
		}
		
	}
	
	if(g_Rx1Flag==UART_RX_BUSY)
	{
		if(++g_Rx1Timer>5)
		{
			g_Rx1Flag=UART_RX_READY;
			frm_set_event(EVENT_UART_ID);
		}
	}
}





