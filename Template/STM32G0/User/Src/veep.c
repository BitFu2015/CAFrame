//veep.c
#include "driver.h"

//**-----------------------------------------------------------------------------------------------FLASH 模拟 EEPROM 功能模块
//FLASH 32K空间中的最后一页（2048字节）
#define EEPROM_START_ADDR 0X8007800   //模拟EEPROM在FLASH中的开始地址
#define EEPROM_END_ADDR   0X8008000		//结束地址
#define EEPROM_BLOCK_SIZE 512		//要存储的参数长度(此值必须能够整除2048)

//向模拟EEPROM写入数据 条件:size<=EEPROM_BLOCK_SIZE
void EepWrite(uint8_t *buf,uint32_t size)
{
	uint8_t *p;
	uint32_t i;
	uint32_t addr = EEPROM_START_ADDR;
	
	FLASH_Unlock();
	
	//查找最后存储地址
	while(addr < EEPROM_END_ADDR)
	{
		p=(uint8_t *)addr;
		for(i=0;i<EEPROM_BLOCK_SIZE;i++)
		{	
			if(p[i]!=0xff) //不空
				break;
		}
		
		if(i==EEPROM_BLOCK_SIZE) //全部为空
			break;
		
		addr+=EEPROM_BLOCK_SIZE;	
	}
	
	//用完空间
	if(addr >= EEPROM_END_ADDR)
	{
		FLASH_PageErase(EEPROM_START_ADDR);
		addr = EEPROM_START_ADDR;
	}

	for(i=0;i<EEPROM_BLOCK_SIZE;i+=8)
	{
		if(i>=size) break;

		FLASH_Program_DoubleWord(addr, buf+i);
		addr+=8;
	}
	
	FLASH_Lock();
}

//从模拟EEPROM读入数据
void EepRead(uint8_t *buf,uint32_t size)
{
	uint32_t addr=EEPROM_START_ADDR;
	uint32_t i;
	uint8_t *p=(uint8_t *)addr;
	
	//查找最后存储地址
	while(addr < EEPROM_END_ADDR)
	{
		p=(uint8_t *)addr;
		for(i=0;i<EEPROM_BLOCK_SIZE;i++)
		{	
			if(p[i]!=0xff) //不空
				break;
		}
		
		if(i==EEPROM_BLOCK_SIZE) //找到一个空位置
			break;
		
		addr+=EEPROM_BLOCK_SIZE;	
	}
	
	if(addr != EEPROM_START_ADDR)
	{
		addr-=EEPROM_BLOCK_SIZE; //调整到最后有效地址处
	}
	
	p=(uint8_t *)addr;
	size=size<EEPROM_BLOCK_SIZE?size:EEPROM_BLOCK_SIZE;
	for(i=0;i<size;i++)
		buf[i]=p[i];
}

//**---------------------------------------------------------------------------------------------//
