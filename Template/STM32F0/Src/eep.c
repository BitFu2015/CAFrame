// eep.c - at24cxx驱动

#include "driver.h"

#define IIC_START()  iic_start()
#define IIC_READ_BYTE(X) iic_readbyte(X)
#define IIC_WRITE_BYTE(X) iic_writebyte(X)
#define IIC_STOP()  iic_stop()

#define DEV_ADDRESS 0xA0		//器件IIC地址

static void eep_busy_wait(void)
{
	uint8_t i,j=0;
	//检测EEPROM是否忙
	while(j<100)
	{
		IIC_START();
		i=IIC_WRITE_BYTE(DEV_ADDRESS); //写	
		if(i==IIC_ACK)
			break;
		j++;
	}
	IIC_STOP();
}
//以下为接口函数////////////////////////////////////////

//AT24CXX通用写页函数
void eep_write_page(uint16_t addr,uint8_t *buf)
{
	uint8_t i;

	//写一页
	IIC_START();
	IIC_WRITE_BYTE(DEV_ADDRESS);//写地址
	IIC_WRITE_BYTE((uint8_t)(addr>>8));
	IIC_WRITE_BYTE((uint8_t)addr);
	
	for(i=0;i<EEP_PAGE_SIZE;i++)
		IIC_WRITE_BYTE(buf[i]);
		
	IIC_STOP();	   
	
	eep_busy_wait();
}

//AT24CXX通用写一字节函数
void eep_write_byte(uint16_t addr,uint8_t dat)
{
	//写一字节
	IIC_START();
	IIC_WRITE_BYTE(DEV_ADDRESS);	//写地址
	IIC_WRITE_BYTE((uint8_t)(addr>>8));
	IIC_WRITE_BYTE((uint8_t)addr);
	IIC_WRITE_BYTE(dat);
		
	IIC_STOP();

	eep_busy_wait();
}


//AT24CXX通用连续读取函数
void eep_read(uint16_t addr,uint8_t *buf,uint8_t len)
{
	uint8_t i;

	IIC_START();
	IIC_WRITE_BYTE(DEV_ADDRESS);
	IIC_WRITE_BYTE((uint8_t)(addr>>8));
	IIC_WRITE_BYTE((uint8_t)addr);
	IIC_START();

	IIC_WRITE_BYTE(DEV_ADDRESS|1);
	for(i=0;i<len-1;i++)
		buf[i]=IIC_READ_BYTE(IIC_ACK);	//ack read
	
	buf[i]=IIC_READ_BYTE(IIC_NOT_ACK);	//NO ACK
	IIC_STOP();	
}

//写连续的多个字节
void eep_write(uint16_t addr,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)
		eep_write_byte(addr++,buf[i]);
}


