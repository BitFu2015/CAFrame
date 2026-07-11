//driver.c - CAFrame工程模板 驱动程序STM32F0单片机示例 

#include "CAFrame.h"

//1ms定时器
volatile uint32_t g_SysTick;

/****************************************************************************
//框架通用接口函数(框架要求在驱动中实现)
//注：函数原型已在CAFrame.h中声明
****************************************************************************/

//初始化系统滴答时钟
void InitTick(void)
{
	g_SysTick=0;	 

	SysTick_Config(36000);	
	LL_SetSystemCoreClock(36000000);
}

//系统定时器中断函数(1ms中断)
void SysTick_Handler( void )
{
	g_SysTick++;
	frm_inc_timer_tick();		
}

static void GPIO_Init(void)
{
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  //GPIO Ports Clock Enable
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
	
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_ALL);
	LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_ALL);
	LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_ALL);
	LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_ALL);

	
  //LED
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	//O0~O7
	GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_3|LL_GPIO_PIN_4|LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7|LL_GPIO_PIN_8|LL_GPIO_PIN_9;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
  //IN0,IN1
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull=LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	//未用到的设置为输出0
	GPIO_InitStruct.Pin = LL_GPIO_PIN_0|LL_GPIO_PIN_1;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_13|LL_GPIO_PIN_14|LL_GPIO_PIN_15;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}


//设备初始化
void drv_init(void)
{
	LL_GPIO_AF_DisableRemap_SWJ();//DISABLE: JTAG-DP Disabled and SW-DP Disabled	
	
	SEI();
	GPIO_Init();
	
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);//分组 抢占0~15(值越高优先级越低)   响应:只能为0
  //SysTick_IRQn interrupt configuration 
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),PRIORITY_L, PRIORITY_SUB));
		
	InitTick();

	LED_OFF;

	eep_init();
	Uart1Init(9600);
	Uart2Init(9600);
	Uart3Init(9600);
	SerialTimerInit();
	
	//启用WDT	
	LL_IWDG_Enable(IWDG);
	LL_IWDG_EnableWriteAccess(IWDG);
	LL_IWDG_SetPrescaler(IWDG,LL_IWDG_PRESCALER_64);
	LL_IWDG_SetReloadCounter(IWDG,500);//0.8秒
	while(LL_IWDG_IsReady(IWDG) != 1){}
	LL_IWDG_ReloadCounter(IWDG);
}

//进入元子操作模式
void drv_enter_critical(void)
{
	CLI();
}
//退出元子操作模式
void drv_exit_critical(void)
{
	SEI();
}

//看门狗复位
void drv_wdt_reset(void)
{
	//复位看门狗
	//IWDG_ReloadCounter();
	LL_IWDG_ReloadCounter(IWDG);
}

//输出控制接口函数
void drv_output(uint8_t port,uint8_t val)
{
	if(val)
	{
		switch(port)
		{
			case IO_Y1:
				LL_GPIO_SetOutputPin(GPIOA , LL_GPIO_PIN_15);
				break;
			case IO_Y2:
				LL_GPIO_SetOutputPin(GPIOB , LL_GPIO_PIN_3);
				break;
			case IO_Y3:
				LL_GPIO_SetOutputPin(GPIOB , LL_GPIO_PIN_4);
				break;
			case IO_Y4:
				LL_GPIO_SetOutputPin(GPIOB , LL_GPIO_PIN_5);
				break;
			case IO_Y5:
				LL_GPIO_SetOutputPin(GPIOB , LL_GPIO_PIN_6);
				break;
			case IO_Y6:
				LL_GPIO_SetOutputPin(GPIOB , LL_GPIO_PIN_7);
				break;
			case IO_Y7:
				LL_GPIO_SetOutputPin(GPIOB , LL_GPIO_PIN_8);
				break;
			case IO_Y8:
				LL_GPIO_SetOutputPin(GPIOB , LL_GPIO_PIN_9);
				break;
			default:
				break;
		}
	}
	else
	{
		switch(port)
		{
			case IO_Y1:
				LL_GPIO_ResetOutputPin(GPIOA , LL_GPIO_PIN_15);
				break;
			case IO_Y2:
				LL_GPIO_ResetOutputPin(GPIOB , LL_GPIO_PIN_3);
				break;
			case IO_Y3:
				LL_GPIO_ResetOutputPin(GPIOB , LL_GPIO_PIN_4);
				break;
			case IO_Y4:
				LL_GPIO_ResetOutputPin(GPIOB , LL_GPIO_PIN_5);
				break;
			case IO_Y5:
				LL_GPIO_ResetOutputPin(GPIOB , LL_GPIO_PIN_6);
				break;
			case IO_Y6:
				LL_GPIO_ResetOutputPin(GPIOB , LL_GPIO_PIN_7);
				break;
			case IO_Y7:
				LL_GPIO_ResetOutputPin(GPIOB , LL_GPIO_PIN_8);
				break;
			case IO_Y8:
				LL_GPIO_ResetOutputPin(GPIOB , LL_GPIO_PIN_9);
				break;
			default:
				break;
		}		
	}
	
}

uint16_t drv_output_status(uint8_t port)
{
	uint16_t ret=0;
	switch(port)
		{
			case IO_Y1:
				if(LL_GPIO_IsOutputPinSet(GPIOA , LL_GPIO_PIN_15)) ret=1;
				break;
			case IO_Y2:
				if(LL_GPIO_IsOutputPinSet(GPIOB , LL_GPIO_PIN_3)) ret=1;
				break;
			case IO_Y3:
				if(LL_GPIO_IsOutputPinSet(GPIOB , LL_GPIO_PIN_4)) ret=1;
				break;
			case IO_Y4:
				if(LL_GPIO_IsOutputPinSet(GPIOB , LL_GPIO_PIN_5)) ret=1;
				break;
			case IO_Y5:
				if(LL_GPIO_IsOutputPinSet(GPIOB , LL_GPIO_PIN_6)) ret=1;
				break;
			case IO_Y6:
				if(LL_GPIO_IsOutputPinSet(GPIOB , LL_GPIO_PIN_7)) ret=1;
				break;
			case IO_Y7:
				if(LL_GPIO_IsOutputPinSet(GPIOB , LL_GPIO_PIN_8)) ret=1;
				break;
			case IO_Y8:
				if(LL_GPIO_IsOutputPinSet(GPIOB , LL_GPIO_PIN_9)) ret=1;
				break;
			default:
				break;
		}
	return ret;
}


//获取输入检测结果
uint8_t drv_input(uint8_t port)
{
	uint8_t tmp;
	
	if(port==IO_X1)
	{
		if(LL_GPIO_IsInputPinSet(GPIOA,LL_GPIO_PIN_0))
			tmp=0;
		else
			tmp=1;
	}
	else if(port == IO_X2)
	{
		if(LL_GPIO_IsInputPinSet(GPIOA,LL_GPIO_PIN_1))
			tmp=0;
		else
			tmp=1;	
	}
	else
		tmp=0;
		
	return tmp;
}

/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//框架标准化接口函数(框架本身不调用)
//注：函数原型已在CAFrame.h中声明
****************************************************************************/
//获取当前定时计数值
UINT drv_get_current_tick(void)
{
	uint32_t ret;
	CLI();
	ret=g_SysTick;
	SEI();
	return ret;
}
//计算过去时间
UINT drv_get_past_tick(UINT t)
{
	uint32_t cur=drv_get_current_tick();

	if(cur>=t)
		return (cur-t);

	return (cur+(0xffffffff-t));
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

//在单独文件中实现：serial.c 24cxx.c




