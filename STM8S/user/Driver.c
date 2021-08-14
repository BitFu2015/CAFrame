/*********************************************
	文件名：Driver.c
	CAS727  CAFrame驱动层程序 
	STM8S103F3  12M外部晶振
	
	芯艺设计室(http://www.chipart.cn)  版权所有
	
	历史记录
	芯艺  2017-6-16  创建
*********************************************/
#include "caframe.h"

/****************************************************************************
//通用接口函数
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
			IWDG->KR=0XAA;//复位看门狗
	}
} 

//1ms定时中断
INTERRUPT void TIM1_IRQHandler (void)
{
	TIM1->SR1 =0;
	
	frm_inc_timer_tick(); //caframe时钟
	
	if(g_DelayTimer)
		g_DelayTimer--;
}

//1ms系统定时器
void Timer1Open(void)
{
	//框架时钟1ms(定时器1)初始化			
  CLK->PCKENR1 |= (1<<7);//TIM1定时器时钟允许
  TIM1->ARRH = (uint8_t)(1000>>8);//自动装载值
  TIM1->ARRL = (uint8_t)1000;
  TIM1->PSCRH = 0;//12分频
  TIM1->PSCRL = 12;
  TIM1->CR1 = TIM1_CR1_ARPE|TIM1_CR1_CEN;//向上计数自动更新开始计数
  TIM1->RCR = 0;	 
	TIM1->IER |= TIM1_IER_UIE;//中断允许	
}

//设备初始化
void drv_init(void)
{
	//时钟配置
		
	//外部晶振
	CLK->CKDIVR=0;//不分频
	CLK->ECKR = 0x01; //启动外部高速晶体振荡器
	while((CLK->ECKR & 0x02) == 0x00); // 等待外部高速振荡器准备好
	CLK->SWCR = CLK->SWCR | 0x02; // 主时钟源切换到外部的高速晶体振荡器
	CLK->SWR = 0xB4; // 选择芯片外部的高速振荡器为主时钟
	while((CLK->SWCR & 0x08) == 0); // 等待切换成功
	CLK->SWCR = CLK->SWCR & 0xFD; // 清除切换标志

	//内部16M RC时钟2分频，主时钟为8M  CPU时钟不分频即：8M  ,外设时钟为主时钟8M 
	//CLK->CKDIVR=0X08;//hsi=16/2=8MHz  CPU:不分频
	
	//LED口初始化
	GPIOD->DDR|=(1<<2)|(1<<3);
	GPIOD->ODR|=(1<<2)|(1<<3); //初始输出高
	GPIOD->CR1|=(1<<2)|(1<<3);//推挽输出
	GPIOD->CR2|=(1<<2)|(1<<3);//10MHz
	
	GPIOD->DDR|=(1<<4);
	GPIOD->ODR&=~(1<<4); //初始输出低
	GPIOD->CR1|=(1<<4);//推挽输出
	GPIOD->CR2|=(1<<4);//10MHz
	
	//按键IO初始化
	/*
	GPIOC->DDR&=~(1<<4);
	GPIOC->CR1|=(1<<4);//上拉打开
	GPIOC->CR2&=~(1<<4);*/
	
	Timer1Open();//框架时钟1ms(定时器1)初始化

	ser_init();
	
	//启用WDT复位最长时间 64k/256/256= 1.02秒
#if WDT_ENABLE>0
	IWDG->KR=0xcc;//使能
	IWDG->KR=0x55;//访问寄存器使能
	IWDG->PR=6;//256分频
	IWDG->RLR=0XFF;//重装计数值
	IWDG->KR=0XAA;//复位看门狗	
#endif
	
	//总中断允许
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
	IWDG->KR=0XAA;//复位看门狗
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

//获取输入检测结果
uint8_t drv_input(uint8_t port)
{
	uint8_t ret=0;
	
		
	return ret;
}

/////////////////////////////////////////////////////////////////////////////


/****************************************************************************
//EEPROM操作驱动
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
		FLASH->DUKR = 0xae; // 写入第一个密钥
		FLASH->DUKR = 0x56; // 写入第二个密钥
	} while((FLASH->IAPSR & 0x08) == 0); // 若解锁未成功，则重新再来

	
	for(i=0;i<size;i++)
	{
		*p=buf[i];
		while((FLASH->IAPSR & 0x04) == 0); // 等待写操作成功
		p++;
	}
}
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//UART1打印输出驱动
****************************************************************************/
#if 0
void UART1_Init(void)
{
	uint16_t uart_div=12000000/9600; //晶振频率/波特率
	uint8_t tmp;
	
	CLK->PCKENR1 |= (1<<2);//UART时钟允许
	
	tmp=uart_div;
	tmp&=0x0f;
	tmp|=(uart_div>>8)&0xf0;
	UART1->BRR1=uart_div>>4;
	UART1->BRR2=tmp;
	
	UART1->CR2=UART1_CR2_TEN|UART1_CR2_REN;//|UART1_CR2_RIEN;//发送接收使能 接收中断使能
	UART1->CR1=0;//UART使能
	
	GPIOD->DDR|=(1<<5);//推挽输出
	GPIOD->CR1|=(1<<5);
	GPIOD->CR2|=(1<<5);
	GPIOD->CR1|=(1<<6);//上拉输入
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
//串口(UART)驱动
****************************************************************************/
HQUEUE g_RxQueue; //接收数据队列
HQUEUE g_TxQueue; //发送数据队列
@tiny uint8_t g_RxBuffer[SERIAL_RECV_BUFFER_SIZE];	//接收缓冲
@tiny uint8_t g_TxBuffer[SERIAL_SEND_BUFFER_SIZE];	//发送缓冲
uint8_t g_RecvWaitCounter;//接收确认时间计数器

//定时器4设置为1ms定时器
void ser_timer_init(void)
{
		//TIM4定时器时钟允许
	  CLK->PCKENR1 |= (1<<4);
		
    TIM4->ARR = 93;//自动装载值
		
		TIM4->PSCR=7;//分频128

    //向上计数自动更新开始计数
    TIM4->CR1 = TIM4_CR1_ARPE|TIM4_CR1_CEN; 
}

void ser_timer_enable(void)
{
	g_RecvWaitCounter=0;
	TIM4->SR1=0;								//清中断标记
	TIM4->IER|=TIM4_IER_UIE; 		//中断允许
}

void ser_timer_disable(void)
{
	TIM4->IER&=~TIM4_IER_UIE;//中断禁止 
}

//定时器中断 ，表示帧接收完成
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
	//uart硬件初始化 
	uint16_t uart_div=12000000/SERIAL_BAUD; //晶振频率/波特率
	uint8_t tmp;
	
	//CLK->PCKENR1 |= (1<<2);//UART1时钟允许
	
	tmp=uart_div;
	tmp&=0x0f;
	tmp|=(uart_div>>8)&0xf0;
	UART1->BRR1=uart_div>>4;
	UART1->BRR2=tmp;
	
	UART1->CR2=UART1_CR2_TEN|UART1_CR2_REN|UART1_CR2_RIEN;//发送接收使能 接收中断使能
	UART1->CR1=0;//UART使能
	
	//发送/接收队列初始化
	QueueCreate(&g_RxQueue,g_RxBuffer,SERIAL_RECV_BUFFER_SIZE);
	QueueCreate(&g_TxQueue,g_TxBuffer,SERIAL_SEND_BUFFER_SIZE);
	
	//数据包判定定时器初始化并停止
	ser_timer_init();
	ser_timer_disable();
}

INTERRUPT void UART1_RX_Handler(void)
{
	uint8_t recv=UART1->DR;
	if(frm_get_event_state(EVENT_UART_ID))//上次数据包还未处理
		return ;
	QueueInputFromISR(&g_RxQueue,recv);
	ser_timer_enable();
}

//发送中断
INTERRUPT void UART1_TX_Handler(void)
{
	if(UART1->SR & UART1_SR_TC)//发送完成
	{
		UART1->CR2&=~UART1_CR2_TCIEN;//发送完成中断禁用
	}
	else //发送寄存器空
	{
		if(QueueGetSize(&g_TxQueue)>0)
			UART1->DR=QueueOutputFromISR(&g_TxQueue);	
		else
		{
			UART1->CR2|=UART1_CR2_TCIEN; //发送完成中断使能
			UART1->CR2&=~UART1_CR2_TIEN; //发送寄存器空中断禁止
		}
	}
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

	i=UART1->SR;//此一步为清发送完成标记
	UART1->DR=buf[0];//发送第一个字节
	UART1->CR2|=UART1_CR2_TIEN;//发送寄存器空中断允许
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
//
****************************************************************************/

/////////////////////////////////////////////////////////////////////////////



