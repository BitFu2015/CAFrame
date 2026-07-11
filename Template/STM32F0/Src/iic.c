//2016-03-16 佟长福 软件模拟iic

#include "driver.h"

///////////////  IIC  //////////////////////////////
//IIC端口操作宏
#define  IIC2_SDA LL_GPIO_IsInputPinSet(GPIOA,LL_GPIO_PIN_10)
#define  IIC2_SDA_H  LL_GPIO_SetOutputPin(GPIOA , LL_GPIO_PIN_10)
#define  IIC2_SDA_L  LL_GPIO_ResetOutputPin(GPIOA , LL_GPIO_PIN_10)          
#define  IIC2_SCL_H LL_GPIO_SetOutputPin(GPIOA , LL_GPIO_PIN_9)
#define  IIC2_SCL_L LL_GPIO_ResetOutputPin(GPIOA , LL_GPIO_PIN_9) 

//
static void iic_delaybus(void)
{
	uint8_t i;
	for(i=0;i<10;i++)
		__nop();
}

void iic_init(void)
{
		//DIR,OUT,LED 初始化
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_9);
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_10);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

//时钟线保持高时数据线从高变低为开始信号
void iic_start(void)
{
	IIC2_SDA_H;   		
	IIC2_SCL_H;
	iic_delaybus();
	IIC2_SDA_L;   				
	iic_delaybus();
	IIC2_SCL_L;   			
	iic_delaybus();	
}

//时钟线保持高时数据线从低变高为停止信号
void iic_stop(void)
{	
	IIC2_SDA_L;		
	IIC2_SCL_H;
	iic_delaybus();
	IIC2_SDA_H;  	
	iic_delaybus();

}
//向IIC总线发送一字节数据并返回是否有ACK应答
uint8_t  iic_writebyte(uint8_t c)
{
	uint8_t i,ack;

	for(i=0;i<8;i++)  
	{
		if(c&0x80)
			IIC2_SDA_H;   
		else  
			IIC2_SDA_L;                
		IIC2_SCL_H;       
		iic_delaybus();        
		IIC2_SCL_L; 
		c<<=1;
		iic_delaybus();
	}
	
	IIC2_SDA_H;	//设置为输入
	iic_delaybus();	 
 	IIC2_SCL_H;
	iic_delaybus();	//等待数据稳定
	if(IIC2_SDA)	//读数据
		ack=IIC_NOT_ACK;     //失败
	else 
		ack=IIC_ACK;        
	IIC2_SCL_L;
	iic_delaybus();//函数尾部确保有延时

	return ack;
}
//从IIC总线读一字节
uint8_t  iic_readbyte(uint8_t ack)
{
	uint8_t i,ret=0;

	IIC2_SDA_H; //确保输入状态
	for(i=0;i<8;i++)
	{                
		IIC2_SCL_H;                
		iic_delaybus();
		ret<<=1;
		if(IIC2_SDA)
			ret++; 
		IIC2_SCL_L;
		iic_delaybus(); 
	}

	if(ack==IIC_ACK)	//应答
		IIC2_SDA_L;     	     
	else		// 非应答
		IIC2_SDA_H;
	iic_delaybus();
	IIC2_SCL_H;
	iic_delaybus();                  
	IIC2_SCL_L;                     
	iic_delaybus();	 //函数尾部确保有延时

	return(ret);
}  
