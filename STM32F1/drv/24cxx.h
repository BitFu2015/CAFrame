//24cxx.h

#ifndef AT24CXX_H
#define AT24CXX_H

/** \defgroup at24c32_51  AT24C32/64/128/256/512���д洢��C���Խӿڳ���[MCS51]

	\par ����
		������������AT24c32 AT24C64 AT24C128 AT24C256 AT24C512������\n

	\par ���Ի���
	��Ƭ����AT89S52 \n	
	ʱ��  : 11.0592MHz		\n
	����  : Keil C51 Compiler/Assembler Kit Version:9.00 \n
		
	\par ���Ե�·
	<img src=".\pic\at24cxx\at24c32sch.jpg">
	
	\par ���Դ���
	\code
		#include <reg52.h>
		#include <string.h>
		
		#include "stdint.h"  //�������ͷ��Ŷ���
		#include "24cxx.h"
		
		void delay_ms(uint16_t t)
		{
			uint16_t i,j;
			for(i=0;i<t;i++)
				for(j=0;j<240;j++);
		}
		
		void UartInit()
		{
			TMOD=0x20; //��ʱ��1������ģʽ2���ڲ���������
		
			TL1=0xFD;  //������9600
			TH1=0xFD;
		
			TR1=1;	   //��ʱ��1����
		
			SCON=0x50; //01010000  ,8λUART
		}
		
		uint8_t g_buf[32];
		
		//���п�дһ�ֽ�
		void UartWriteByte(uint8_t c)
		{
			SBUF=c;
			while(TI==0); //һֱ�ȴ�������ɱ�־��λ,˵���������.
			TI=0;		  //�������ʽ����˱�־λ
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
			
			//���͵�����
			for(i=0;i<16;i++)
				UartWriteByte(g_buf[i]);
			
			while(1);
		}	
	\endcode


*/

/** \ingroup at24c32_51
	\brief �����ͺŶ���
	
	AT24C32,AT24C64,AT24C128,AT24C256,AT24C512��ѡ��һ
*/
#define AT24C256	   //�˴����������ͺ�

#if defined AT24C32 || defined AT24C64
	#define EEP_PAGE_SIZE 32
#elif defined AT24C128 || defined AT24C256 
	#define EEP_PAGE_SIZE 64
#elif defined AT24C512
	#define EEP_PAGE_SIZE 128
#else
	#warning("24cxx.h�ļ��������ͺ�δ����!")
#endif

/** \ingroup at24c32_51
	\brief ��EEPROM�洢����ȡ����
	
	\param addr EEPROM�����ݵ��ֽ�Ϊ��λ�����Ե�ַ\n
	����AT24C32 addr��Χ��0~4095 \n
	����AT24C64 addr��Χ��0~8191 \n
	����AT24C128 addr��Χ��0~16383 \n
	����AT24C256 addr��Χ��0~32767 \n
	����AT24C512 addr��Χ��0~65535  
	\param buf ��ȡ���������˻��������ȱ�����ڻ���ڲ���lenָ���ĳ���
	\param len ��ȡ����	
*/
void eep_read(uint16_t addr,uint8_t *buf,uint8_t len);

/** \ingroup at24c32_51
	\brief д���ݵ�EEPROM�洢��
	
	\param addr EEPROM�����ݵ��ֽ�Ϊ��λ�����Ե�ַ\n
	����AT24C32 addr��Χ��0~4095 \n
	����AT24C64 addr��Χ��0~8191 \n
	����AT24C128 addr��Χ��0~16383 \n
	����AT24C256 addr��Χ��0~32767 \n
	����AT24C512 addr��Χ��0~65535 
	\param buf д�����ݻ��������˻��������ȱ�����ڻ���ڲ���lenָ���ĳ���
	\param len ��ȡ����	
*/
void eep_write(uint16_t addr,uint8_t *buf,uint8_t len);

/** \ingroup at24c32_51
	\brief дһҳ���ݵ�EEPROM�洢�� 
	
	�˺���д��Ч�ʸ���eep_write��������������Ҫ��ҳ��֯���˺���һ�οɽ�һҳ����д�뵽�洢��
	���ڲ�ͬ������ҳ��С��ͬ��ҳ��С������ɷ���EEP_PAGE_SIZE ��������:
	����AT24C32 EEP_PAGE_SIZE ���� 32
	����AT24C64 EEP_PAGE_SIZE ���� 32
	����AT24C128 EEP_PAGE_SIZE ���� 64
	����AT24C256 EEP_PAGE_SIZE ���� 64
	����AT24C512 EEP_PAGE_SIZE ���� 128 
	
	\param addr EEPROM�洢��ҳ�׵�ַ
	= EEP_PAGE_SIZE * n (n = 0,1,2,3 ...)
	\param buf д�����ݻ���������������С����EEP_PAGE_SIZE	
*/
void eep_write_page(uint16_t addr,uint8_t *buf); 

void eep_init(void);

#endif
