//driver.c - CAFrame工程模板 驱动程序STM32F0单片机示例 

#include "CAFrame.h"

/****************************************************************************
//框架通用接口函数(框架要求在驱动中实现)
//注：函数原型已在CAFrame.h中声明
****************************************************************************/
volatile static uint32_t g_SysTick=0;

//系统定时器中断 1ms中断
void SysTick_Handler(void)
{
	g_SysTick++;
  frm_inc_timer_tick();
}


//看门狗设置
static void WDG_Config(void)
{
    LL_IWDG_EnableWriteAccess(IWDG);

    //设置预分频，例如 LL_IWDG_PRESCALER_32 → 实际分频 = 4×2^prer = 128
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_32);   // 32kHz/128 = 250 Hz

    //设置 12bit 重装载值（0~4095）
    LL_IWDG_SetReloadCounter(IWDG, 250);                // 250/250Hz = 1 s

    //等待硬件把 PR/RLR 搬完（FVU 位清掉） 
    while (LL_IWDG_IsReady(IWDG) == 0) {}

    LL_IWDG_ReloadCounter(IWDG);

    LL_IWDG_Enable(IWDG);
}

//设备初始化
void drv_init(void)
{	
	//DIR,OUT,LED 初始化
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  //
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);

  //输出IO
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4|LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  //LED
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//KEY
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	iic_init();
	drv_serial_init();

	//系统时钟1ms
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	
	WDG_Config();//看门狗1秒计数
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

//输出控制接口函数
void drv_output(uint8_t port,uint8_t val)
{
	if(val)
	{
		if(port==IO_Y1)
			LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
		else if(port==IO_Y2)
			LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6);
		else;
	}
	else
	{
		if(port==IO_Y1)
			LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
		else if(port == IO_Y2)
			LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);
		else;
	}
}

//获取输入检测结果
uint8_t drv_input(uint8_t port)
{
	uint8_t ret=0;

	if(port==IO_X1)
	{
			ret=LL_GPIO_IsInputPinSet(GPIOA,LL_GPIO_PIN_0);
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

UINT drv_get_past_tick(UINT t)
{
	uint32_t tmp=g_SysTick;
	return tmp-t;
}

void drv_eeprom_read(uint16_t addr,uint8_t *buf,uint8_t size)
{
	eep_read(addr,buf,size);
}

void drv_eeprom_write(uint16_t addr,uint8_t *buf,uint8_t size)
{
	eep_write(addr,buf,size);
}
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//用户扩展驱动
//注：用户扩展驱动接口函数原型需要在driver.h中声明，以便在APP中调用
****************************************************************************/

//*----------------------------------------------------------------------UART
#define RS485_RX_SEL LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4)
#define RS485_TX_SEL LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4)

#define UART_BUF_SIZE SERIAL_BUFF_SIZE
uint8_t g_UartRxBuffer[UART_BUF_SIZE];//接收缓冲区
uint8_t g_UartTxBuffer[UART_BUF_SIZE];//发送缓冲区
volatile uint8_t g_UartTxIndex;
volatile uint8_t g_UartRxIndex;
uint8_t g_UartTxSize;

void TIM3_Config(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  //Peripheral clock enable 
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  // TIM3 interrupt Init
  NVIC_SetPriority(TIM3_IRQn, 0);
  NVIC_EnableIRQ(TIM3_IRQn);

  TIM_InitStruct.Prescaler = 47;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 5000;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM3, &TIM_InitStruct);
	
  LL_TIM_DisableARRPreload(TIM3);
  LL_TIM_SetClockSource(TIM3, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM3, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM3);

	
	LL_TIM_EnableIT_UPDATE(TIM3);
}

//TIM3中断处理函数
void TIM3_IRQHandler(void)
{
	LL_TIM_ClearFlag_UPDATE(TIM3);

	frm_set_event(EVENT_UART_ID);
}

void UartTimerEnable(void)
{	
	LL_TIM_DisableCounter(TIM3);
	TIM3->CNT=0;
	LL_TIM_EnableCounter(TIM3);
}

void drv_serial_init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  //USART1时钟使能
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);

  //PA2   ------> USART1_TX
  //PA3   ------> USART1_RX
  
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // USART1 interrupt Init 
  NVIC_SetPriority(USART1_IRQn, 0);
  NVIC_EnableIRQ(USART1_IRQn);

  USART_InitStruct.BaudRate = 9600;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_DisableIT_CTS(USART1);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);
	LL_USART_EnableIT_RXNE(USART1);
	
	TIM3_Config(); //接收数据包检测计数器配置
	
	g_UartTxIndex=0;
	g_UartTxSize=0;
	g_UartRxIndex=0;
}

//USART1中断处理
void USART1_IRQHandler(void)
{
	uint8_t c;
	uint16_t isr=USART1->ISR;
	
	//接收到一个字节
	if((isr & USART_ISR_RXNE) == USART_ISR_RXNE)
	{
		c=(uint8_t)(USART1->RDR);
		if(g_UartRxIndex < UART_BUF_SIZE)
			g_UartRxBuffer[g_UartRxIndex++]=c;
		UartTimerEnable();
	}
	else if((isr & USART_ISR_TC) == USART_ISR_TC)//发送完成
	{
		LL_USART_DisableIT_TC(USART1);//发送完成,关闭中断
		LL_USART_ClearFlag_TC(USART1);

		RS485_RX_SEL;
	}
	else	if((isr & USART_ISR_TXE) == USART_ISR_TXE)//发送寄存器空
	{
		if(g_UartTxIndex<g_UartTxSize)
			 LL_USART_TransmitData8(USART1, g_UartTxBuffer[g_UartTxIndex++]);
		else
		{
			LL_USART_DisableIT_TXE(USART1);	//发送寄存器空中断禁止
			LL_USART_EnableIT_TC(USART1);		//发送完成中断使能
		}
	}
	else;
}

//发送数据包
uint8_t drv_serial_output(uint8_t *buf,uint8_t len)
{
	uint8_t i;
	
	for(i=1;i<len;i++)
		g_UartTxBuffer[i]=buf[i];
	g_UartTxSize=len;
	g_UartTxIndex=1;

	RS485_TX_SEL;
	LL_USART_TransmitData8(USART1, buf[0]);
	
	LL_USART_EnableIT_TXE(USART1);//发送寄存器空中断允许
	return len;
}

//读取缓冲区中接收到的数据包
uint8_t drv_serial_input(uint8_t *buf)
{
	uint8_t i;
	
	for(i=0;i<g_UartRxIndex;i++)
		buf[i]=g_UartRxBuffer[i];
	do{g_UartRxIndex=0;}while(g_UartRxIndex);
	
	return i;
}

/////////////////////////////////////////////////////////////////////////////
