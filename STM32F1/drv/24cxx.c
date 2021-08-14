/*/////////////////////////////////////////////////////////
	AT24C32/64/128/256/512 读写程序
	文件名:24cxx.c
	编译：Keil MDK-ARM 4.12

	单片机:STM32F103C8T6
	时钟: 72MHz主频

	芯艺设计室 CLib库  版权所有  http://www.chipart.cn	

	注：本程序有偿提供，严禁任何形式公开发布！！！       */
///////////////////////////////////////////////////////////
#include"caframe.h"

void eep_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	
	// Configure I2C1 pins: SCL and SDA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;//开漏输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
}


#define  IIC_SDA GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)
#define  IIC_SDA_H  GPIO_SetBits(GPIOB , GPIO_Pin_9)
#define  IIC_SDA_L  GPIO_ResetBits(GPIOB , GPIO_Pin_9)          
#define  IIC_SCL_H GPIO_SetBits(GPIOB , GPIO_Pin_8)
#define  IIC_SCL_L GPIO_ResetBits(GPIOB , GPIO_Pin_8) 

#define IIC_ACK			1
#define IIC_NOT_ACK		0
#define DEVICE_ADDRESS 0xA0		//器件IIC地址

//总线延时
static void iic_delaybus()
{
	uint8_t i;
	for(i=0;i<10;i++)
		__nop();
	//DelayUs(1);
}

//时钟线保持高时数据线从高变低为开始信号
static void iic_start()
{
	IIC_SDA_H;   		
	IIC_SCL_H;
	iic_delaybus();
	IIC_SDA_L;   				
	iic_delaybus();
	IIC_SCL_L;   			
	iic_delaybus();	
}

//时钟线保持高时数据线从低变高为停止信号
static void iic_stop()
{	
	IIC_SDA_L;		
	IIC_SCL_H;
	iic_delaybus();
	IIC_SDA_H;  	
	iic_delaybus();

}
//向IIC总线发送一字节数据并返回是否有ACK应答
static uint8_t  iic_writebyte(uint8_t c)
{
	uint8_t i,ack;

	for(i=0;i<8;i++)  
	{
		if(c&0x80)
			IIC_SDA_H;   
		else  
			IIC_SDA_L;                
		IIC_SCL_H;       
		iic_delaybus();        
		IIC_SCL_L; 
		c<<=1;
		iic_delaybus();
	}
	
	IIC_SDA_H;	//设置为输入
	iic_delaybus();	 
 	IIC_SCL_H;
	iic_delaybus();	//等待数据稳定
	if(IIC_SDA)	//读数据
		ack=IIC_NOT_ACK;     //失败
	else 
		ack=IIC_ACK;        
	IIC_SCL_L;
	iic_delaybus();//函数尾部确保有延时

	return ack;
}
//从IIC总线读一字节
static uint8_t  iic_readbyte(uint8_t ack)
{
	uint8_t i,ret=0;

	IIC_SDA_H; //确保输入状态
	for(i=0;i<8;i++)
	{                
		IIC_SCL_H;                
		iic_delaybus();
		ret<<=1;
		if(IIC_SDA)
			ret++; 
		IIC_SCL_L;
		iic_delaybus(); 
	}

	if(ack==IIC_ACK)	//应答
		IIC_SDA_L;     	     
	else		// 非应答
		IIC_SDA_H;
	iic_delaybus();
	IIC_SCL_H;
	iic_delaybus();                  
	IIC_SCL_L;                     
	iic_delaybus();	 //函数尾部确保有延时

	return(ret);
}  

static void eep_busy_wait(void)
{
	uint8_t i,j=0;
	//检测EEPROM是否忙
	while(j<100)
	{
		iic_start();
		i=iic_writebyte(DEVICE_ADDRESS); //写	
		if(i==IIC_ACK)
			break;
		j++;
	}
	iic_stop();
}
//以下为接口函数////////////////////////////////////////

//AT24CXX通用写页函数
void eep_write_page(uint16_t addr,uint8_t *buf)
{
	uint8_t i;

	//写一页
	iic_start();
	iic_writebyte(DEVICE_ADDRESS);//写地址
	iic_writebyte((uint8_t)(addr>>8));
	iic_writebyte((uint8_t)addr);
	
	for(i=0;i<EEP_PAGE_SIZE;i++)
		iic_writebyte(buf[i]);
		
	iic_stop();	   
	
	eep_busy_wait();
}

//AT24CXX通用写一字节函数
void eep_write_byte(uint16_t addr,uint8_t dat)
{
	//写一字节
	iic_start();
	iic_writebyte(DEVICE_ADDRESS);	//写地址
	iic_writebyte((uint8_t)(addr>>8));
	iic_writebyte((uint8_t)addr);
	iic_writebyte(dat);
		
	iic_stop();

	eep_busy_wait();
}


//AT24CXX通用连续读取函数
void eep_read(uint16_t addr,uint8_t *buf,uint8_t len)
{
	uint8_t i;

	iic_start();
	iic_writebyte(DEVICE_ADDRESS);
	iic_writebyte((uint8_t)(addr>>8));
	iic_writebyte((uint8_t)addr);
	iic_start();

	iic_writebyte(DEVICE_ADDRESS|1);
	for(i=0;i<len-1;i++)
		buf[i]=iic_readbyte(IIC_ACK);	//ack read
	
	buf[i]=iic_readbyte(IIC_NOT_ACK);	//NO ACK
	iic_stop();	
}

//写连续的多个字节
void eep_write(uint16_t addr,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)
		eep_write_byte(addr++,buf[i]);
}


