/*/////////////////////////////////////////////////////////
	AT24C32/64/128/256/512 ��д����
	�ļ���:24cxx.c
	���룺Keil MDK-ARM 4.12

	��Ƭ��:STM32F103C8T6
	ʱ��: 72MHz��Ƶ

	о������� CLib��  ��Ȩ����  http://www.chipart.cn	

	ע���������г��ṩ���Ͻ��κ���ʽ��������������       */
///////////////////////////////////////////////////////////
#include"caframe.h"

void eep_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	
	// Configure I2C1 pins: SCL and SDA
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;//��©���
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
}


#define  IIC_SDA GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)
#define  IIC_SDA_H  GPIO_SetBits(GPIOB , GPIO_Pin_9)
#define  IIC_SDA_L  GPIO_ResetBits(GPIOB , GPIO_Pin_9)          
#define  IIC_SCL_H GPIO_SetBits(GPIOB , GPIO_Pin_8)
#define  IIC_SCL_L GPIO_ResetBits(GPIOB , GPIO_Pin_8) 

#define IIC_ACK			1
#define IIC_NOT_ACK		0
#define DEVICE_ADDRESS 0xA0		//����IIC��ַ

//������ʱ
static void iic_delaybus()
{
	uint8_t i;
	for(i=0;i<10;i++)
		__nop();
	//DelayUs(1);
}

//ʱ���߱��ָ�ʱ�����ߴӸ߱��Ϊ��ʼ�ź�
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

//ʱ���߱��ָ�ʱ�����ߴӵͱ��Ϊֹͣ�ź�
static void iic_stop()
{	
	IIC_SDA_L;		
	IIC_SCL_H;
	iic_delaybus();
	IIC_SDA_H;  	
	iic_delaybus();

}
//��IIC���߷���һ�ֽ����ݲ������Ƿ���ACKӦ��
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
	
	IIC_SDA_H;	//����Ϊ����
	iic_delaybus();	 
 	IIC_SCL_H;
	iic_delaybus();	//�ȴ������ȶ�
	if(IIC_SDA)	//������
		ack=IIC_NOT_ACK;     //ʧ��
	else 
		ack=IIC_ACK;        
	IIC_SCL_L;
	iic_delaybus();//����β��ȷ������ʱ

	return ack;
}
//��IIC���߶�һ�ֽ�
static uint8_t  iic_readbyte(uint8_t ack)
{
	uint8_t i,ret=0;

	IIC_SDA_H; //ȷ������״̬
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

	if(ack==IIC_ACK)	//Ӧ��
		IIC_SDA_L;     	     
	else		// ��Ӧ��
		IIC_SDA_H;
	iic_delaybus();
	IIC_SCL_H;
	iic_delaybus();                  
	IIC_SCL_L;                     
	iic_delaybus();	 //����β��ȷ������ʱ

	return(ret);
}  

static void eep_busy_wait(void)
{
	uint8_t i,j=0;
	//���EEPROM�Ƿ�æ
	while(j<100)
	{
		iic_start();
		i=iic_writebyte(DEVICE_ADDRESS); //д	
		if(i==IIC_ACK)
			break;
		j++;
	}
	iic_stop();
}
//����Ϊ�ӿں���////////////////////////////////////////

//AT24CXXͨ��дҳ����
void eep_write_page(uint16_t addr,uint8_t *buf)
{
	uint8_t i;

	//дһҳ
	iic_start();
	iic_writebyte(DEVICE_ADDRESS);//д��ַ
	iic_writebyte((uint8_t)(addr>>8));
	iic_writebyte((uint8_t)addr);
	
	for(i=0;i<EEP_PAGE_SIZE;i++)
		iic_writebyte(buf[i]);
		
	iic_stop();	   
	
	eep_busy_wait();
}

//AT24CXXͨ��дһ�ֽں���
void eep_write_byte(uint16_t addr,uint8_t dat)
{
	//дһ�ֽ�
	iic_start();
	iic_writebyte(DEVICE_ADDRESS);	//д��ַ
	iic_writebyte((uint8_t)(addr>>8));
	iic_writebyte((uint8_t)addr);
	iic_writebyte(dat);
		
	iic_stop();

	eep_busy_wait();
}


//AT24CXXͨ��������ȡ����
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

//д�����Ķ���ֽ�
void eep_write(uint16_t addr,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)
		eep_write_byte(addr++,buf[i]);
}


