//Driver.c　-  CAFrame硬件驱动层实现程序

#include "driver.h"
#include <stdarg.h>

//设备初始化
void drv_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//HCLK=72M  PCLK1(APB1时钟)=36M  PCLK2(APB2时钟)=72M
	//以上由main函数前执行的SystemInit函数配置好
	
//#ifdef BOOT_APP 
	NVIC_SetVectorTable(0x08000000,0x00003000); 
//#endif
	
	//GPIOA GPIOB时钟允许
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC ,ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 

	//禁止JTAG,SWJ
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);

	//LED
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

 	//输出
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//输入按键引脚
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//输入
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_3|GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	LED_OFF;
	
	eep_init();
	
	InitTick();
	
	Uart1Init();
	Uart2Init();
	Uart3Init();
	UartTimerInit();
	
	AdcInit();
	
	//启用WDT	
	//允许写相关寄存器
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	//设置IWDG时钟: 40KHz(LSI) / 256 = 156Hz
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	//设置重新计数值为 156 该参数允许取值范围为 0–0x0FFF
	IWDG_SetReload(312);  //两秒超时
	//IWDG计数器复位
	IWDG_ReloadCounter();
	//使能IWDG (the LSI oscillator will be enabled by hardware)
	IWDG_Enable();	
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
	IWDG_ReloadCounter();
}
/////////////////////////////////////////////////////////////////////////////


/****************************************************************************
//开关输入/输出,此部分为框架必须实现的功能
****************************************************************************/
//输出控制接口函数
void drv_output(uint8_t port,uint8_t val)
{
	if(val)
	{
		switch(port)
		{
			case IO_Y1:
				GPIO_SetBits(GPIOA , GPIO_Pin_11);break;
			case IO_Y2:
				GPIO_SetBits(GPIOA , GPIO_Pin_10);break;
			case IO_Y3:
				GPIO_SetBits(GPIOA , GPIO_Pin_9);break;
			case IO_Y4:
				GPIO_SetBits(GPIOA , GPIO_Pin_8);break;
			case IO_Y5:
				GPIO_SetBits(GPIOB , GPIO_Pin_15);break;
			case IO_Y6:
				GPIO_SetBits(GPIOB , GPIO_Pin_14);break;
			case IO_Y7:
				GPIO_SetBits(GPIOB , GPIO_Pin_13);break;
			case IO_Y8:
				GPIO_SetBits(GPIOA , GPIO_Pin_12);break;
			default:
				break;
		}
	} 
	else
	{
		switch(port)
		{
			case IO_Y1:
				GPIO_ResetBits(GPIOA , GPIO_Pin_11);break;
			case IO_Y2:
				GPIO_ResetBits(GPIOA , GPIO_Pin_10);break;
			case IO_Y3:
				GPIO_ResetBits(GPIOA , GPIO_Pin_9);break;
			case IO_Y4:
				GPIO_ResetBits(GPIOA , GPIO_Pin_8);break;
			case IO_Y5:
				GPIO_ResetBits(GPIOB , GPIO_Pin_15);break;
			case IO_Y6:
				GPIO_ResetBits(GPIOB , GPIO_Pin_14);break;
			case IO_Y7:
				GPIO_ResetBits(GPIOB , GPIO_Pin_13);break;
			case IO_Y8:
				GPIO_ResetBits(GPIOA , GPIO_Pin_12);break;
			default:
				break;
		}
	}
}

uint8_t drv_output_status(uint8_t port)
{
	uint8_t ret=0;
	switch(port)
	{
		case IO_Y1:
			if(GPIOA->ODR & GPIO_Pin_11)
				ret=1;
			break;
		case IO_Y2:
			if(GPIOA->ODR & GPIO_Pin_10)
				ret=1;
			break;
		case IO_Y3:
			if(GPIOA->ODR & GPIO_Pin_9)
				ret=1;
			break;
		case IO_Y4:
			if(GPIOA->ODR & GPIO_Pin_8)
				ret=1;
			break;
		case IO_Y5:
			if(GPIOB->ODR & GPIO_Pin_15)
				ret=1;
			break;			
		case IO_Y6:
			if(GPIOB->ODR & GPIO_Pin_14)
				ret=1;
			break;			
		case IO_Y7:
			if(GPIOB->ODR & GPIO_Pin_13)
				ret=1;
			break;			
		case IO_Y8:
			if(GPIOA->ODR & GPIO_Pin_12)
				ret=1;
			break;			
		default:
			break;
	}
	return ret;
}

//获取输入检测结果
uint8_t drv_input(uint8_t port)
{
	uint8_t tmp=0;
	
	if(port==IO_X1)
	{
		if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_4))
			tmp=1;
	}
	else if(port==IO_X2)
	{
		if(!GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3))
			tmp=1;
	}
	else if(port==IO_X3)
	{
		if(!GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15))
			tmp=1;
	}
	else if(port==IO_X4)
	{
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))
			tmp=1;
	}else;
		
	return tmp;
}

/////////////////////////////////////////////////////////////////////////////

//////////////////////////////标准输出口/////////////////////////////////////
int DebugSendByte(int dt)
{
	uint8_t c=(uint8_t)dt;
	Uart3Post(&c,1);
	return dt;
}
uint8_t hex_char(uint8_t v)
{
	v%=16;
	if(v>=10)
		v='A'+(v-10);
	else
		v+='0';
	return v;
}

void DebugPrintBin(uint8_t *buf,uint8_t len)
{
	uint8_t i,tmp;
	for(i=0;i<len;i++)
	{
		tmp=buf[i]>>4;
		DebugSendByte(hex_char(tmp));
		tmp=buf[i]&0x0f;
		DebugSendByte(hex_char(tmp));
		DebugSendByte(' ');
	}
}

struct __FILE 
{
  int handle; // 
};

FILE __stdout;

int fputc(int ch, FILE *f) 
{
  return (DebugSendByte(ch));
}

//模拟打印输出函数
void myprintf(char *fmt,...)
{
	/*
	va_list ap;

	char strval[6];
	char *p;
	int nval;
	uint8_t i;

	va_start(ap,fmt);
	for(p=fmt;*p;p++)
	{
		if(*p!='%')
		{
			DebugSendByte(*p);
			continue;
		}
		p++;
		if(*p=='d')
		{
			nval=va_arg(ap,int);
			//inttostr(strval,nval);
			for(i=0;strval[i];i++)
				DebugSendByte(strval[i]);
		}
	}
	va_end(ap);	*/
}

/////////////////////////////////////////////////////////////////////////////



