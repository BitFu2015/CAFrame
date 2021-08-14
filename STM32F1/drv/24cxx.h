//24cxx.h

#ifndef AT24CXX_H
#define AT24CXX_H

/** \defgroup at24c32_51  AT24C32/64/128/256/512串行存储器C语言接口程序[MCS51]

	\par 特性
		本程序适用于AT24c32 AT24C64 AT24C128 AT24C256 AT24C512等器件\n

	\par 测试环境
	单片机：AT89S52 \n	
	时钟  : 11.0592MHz		\n
	编译  : Keil C51 Compiler/Assembler Kit Version:9.00 \n
		
	\par 测试电路
	<img src=".\pic\at24cxx\at24c32sch.jpg">
	
	\par 测试代码
	\code
		#include <reg52.h>
		#include <string.h>
		
		#include "stdint.h"  //数据类型符号定义
		#include "24cxx.h"
		
		void delay_ms(uint16_t t)
		{
			uint16_t i,j;
			for(i=0;i<t;i++)
				for(j=0;j<240;j++);
		}
		
		void UartInit()
		{
			TMOD=0x20; //定时器1工作在模式2用于产生波特率
		
			TL1=0xFD;  //波特率9600
			TH1=0xFD;
		
			TR1=1;	   //定时器1工作
		
			SCON=0x50; //01010000  ,8位UART
		}
		
		uint8_t g_buf[32];
		
		//向串行口写一字节
		void UartWriteByte(uint8_t c)
		{
			SBUF=c;
			while(TI==0); //一直等待发送完成标志置位,说明发送完成.
			TI=0;		  //用软件方式清除此标志位
		}
		
		void main()
		{
			uint8_t i;
			
			delay_ms(100);
			UartInit();
			
			//eep_write_page(2048,"1234567890abcdefg");
			eep_write(2048,"1234567890abcdefg",16);
			memset(g_buf,0,20);
			eep_read(2048,g_buf,16);
			
			//发送到串口
			for(i=0;i<16;i++)
				UartWriteByte(g_buf[i]);
			
			while(1);
		}	
	\endcode


*/

/** \ingroup at24c32_51
	\brief 器件型号定义
	
	AT24C32,AT24C64,AT24C128,AT24C256,AT24C512中选其一
*/
#define AT24C256	   //此处定义器件型号

#if defined AT24C32 || defined AT24C64
	#define EEP_PAGE_SIZE 32
#elif defined AT24C128 || defined AT24C256 
	#define EEP_PAGE_SIZE 64
#elif defined AT24C512
	#define EEP_PAGE_SIZE 128
#else
	#warning("24cxx.h文件中器件型号未定义!")
#endif

/** \ingroup at24c32_51
	\brief 从EEPROM存储器读取数据
	
	\param addr EEPROM中数据的字节为单位的线性地址\n
	对于AT24C32 addr范围是0~4095 \n
	对于AT24C64 addr范围是0~8191 \n
	对于AT24C128 addr范围是0~16383 \n
	对于AT24C256 addr范围是0~32767 \n
	对于AT24C512 addr范围是0~65535  
	\param buf 读取缓冲区，此缓冲区长度必须大于或等于参数len指定的长度
	\param len 读取长度	
*/
void eep_read(uint16_t addr,uint8_t *buf,uint8_t len);

/** \ingroup at24c32_51
	\brief 写数据到EEPROM存储器
	
	\param addr EEPROM中数据的字节为单位的线性地址\n
	对于AT24C32 addr范围是0~4095 \n
	对于AT24C64 addr范围是0~8191 \n
	对于AT24C128 addr范围是0~16383 \n
	对于AT24C256 addr范围是0~32767 \n
	对于AT24C512 addr范围是0~65535 
	\param buf 写入数据缓冲区，此缓冲区长度必须大于或等于参数len指定的长度
	\param len 读取长度	
*/
void eep_write(uint16_t addr,uint8_t *buf,uint8_t len);

/** \ingroup at24c32_51
	\brief 写一页数据到EEPROM存储器 
	
	此函数写入效率高于eep_write函数，但数据需要按页组织。此函数一次可将一页数据写入到存储器
	对于不同的器件页大小不同，页大小被定义成符号EEP_PAGE_SIZE 具体如下:
	对于AT24C32 EEP_PAGE_SIZE 等于 32
	对于AT24C64 EEP_PAGE_SIZE 等于 32
	对于AT24C128 EEP_PAGE_SIZE 等于 64
	对于AT24C256 EEP_PAGE_SIZE 等于 64
	对于AT24C512 EEP_PAGE_SIZE 等于 128 
	
	\param addr EEPROM存储器页首地址
	= EEP_PAGE_SIZE * n (n = 0,1,2,3 ...)
	\param buf 写入数据缓冲区，缓冲区大小至少EEP_PAGE_SIZE	
*/
void eep_write_page(uint16_t addr,uint8_t *buf); 

void eep_init(void);

#endif
