/****************************************************************************|
	�ļ�:App.c
	CAFrame Ӧ�ò�ģ�����
	
	о�������(http://www.chipart.cn)  ��Ȩ����
	������ҳ:http://www.chipart.cn/projects/prj_hit.asp?id=8

	MCU :ATMEGA328
	ʱ��:�ⲿ11.0592MHz����
	����:WinAVR20100110

	�޸ļ�¼:
		2019-04-27 ������

*****************************************************************************/

#include "CAFrame.h"
#include <util/crc16.h>


/*---------------------------------------------------------------------------
������03h/10h MODBUS��ַ 	
											0:���״̬�Ĵ���g_HolReg[0]
											1:�豸��ַ��g_HolReg[1]
											2~9:����

---------------------------------------------------------------------------*/
#define MB_BROADCAST_ADDR 0xFA  //�㲥��ַ


#define MODBUS_REG_SIZE 10
uint16_t g_HolReg[MODBUS_REG_SIZE];  //MODBUS�Ĵ��� 
uint8_t g_mbBuf[SERIAL_BUFF_SIZE];    //ͨ���û�����


//CRCУ�����
uint16_t mbCRC16( uint8_t * pucFrame, uint16_t usLen )
{
   uint8_t hi,lo;
   uint16_t i;
   uint16_t crc;
   crc=0xFFFF;
   for (i=0;i<usLen;i++)
   {
     crc= _crc16_update(crc, *pucFrame);
     pucFrame++;
   }
  hi=crc%256;
  lo=crc/256;
  crc=(hi<<8)|lo;
  return crc;
}

uint16_t BufToReg(uint8_t *buf)
{
  uint16_t ret=buf[0];
  ret<<=8;
  return ret+buf[1];
}

void RegToBuf(uint8_t *buf,uint16_t reg)
{
  buf[0]=reg>>8;
  buf[1]=reg;
}

//MODBUS ����� ����
void mbCommand(uint8_t *buf,uint8_t len)
{
	uint8_t i,size,tmp;
	uint16_t addr,crc;
	
	i=g_HolReg[1];
	if((buf[0] != MB_BROADCAST_ADDR)&& (buf[0] != i))	//��ַ����
		return ;	
	
	if(mbCRC16(buf,len)) //У��
		return ;	
		
	addr=BufToReg(buf+2);		
	size=BufToReg(buf+4);	//���Ը��ֽ�	
	if(buf[1]==0x03) 			//������Ĵ��� ������03h
	{
		if(addr+size > MODBUS_REG_SIZE)  	//��Ч��ַ�ͳ���
			return ;
		buf[2]=size*2; 			//���ݳ���
		len=3;
		for(i=0;i<size;i++)
		{
			RegToBuf(buf+len,g_HolReg[addr+i]);
			len+=2;
		}
		crc=mbCRC16(buf,len);
		RegToBuf(buf+len,crc);
		len+=2;
		drv_serial_output(buf,len);
	}
	else if(buf[1]==0x10) //д����Ĵ��� ������10h
	{
		if(addr+size>4)			//���ݵ�ַ�ͳ��Ȳ���
			return ;
		if(buf[6]!=size*2)	//д�볤�Ȳ���
			return ;
		for(i=0;i<size;i++)
			g_HolReg[addr+i]=BufToReg(buf+7+(i*2));
		crc=mbCRC16(buf,6);
		RegToBuf(buf+6,crc);
		drv_serial_output(buf,8);
	}
	else if(buf[1]==0x01)//����Ȧ ������01h
	{
		if((addr<1)||(addr>8))
			return ;
		tmp=addr-1;
		if((size +addr -1)>8)
			return ;
		buf[2]=size;
		for(i=0;i<size;i++)
		{
			if(g_HolReg[0]& (1<<tmp))
				buf[3+i]=1;
			else
				buf[3+i]=0;
			tmp++;
		}
		i+=3;
		crc=mbCRC16(buf,i);
		RegToBuf(buf+i,crc);
		drv_serial_output(buf,i+2);
	}
	else if(buf[1]==0x05)//������Ȧ:������05h
	{
		if((addr<1)||(addr>8))
			return ;
		tmp=addr-1;
		
		if(size == 0xaa) 
			g_HolReg[0]|=1<<tmp;
		else
			g_HolReg[0]&=~(1<<tmp);
			
		drv_serial_output(buf,len);//����ԭ������	
	}
	else;
}

//���ڽ����¼�
void SerialEvent(void)
{
    uint8_t size = drv_serial_input(g_mbBuf);
    mbCommand(g_mbBuf,size); 
    //drv_serial_output(g_mbBuf,size); //������
}

//��ʱ�������״̬
void UpdateOutput(void)
{
	uint8_t i,tmp;
	
	tmp=(uint8_t)g_HolReg[0];

	for(i=0;i<8;i++)
	{
		if(tmp&0x01)
			drv_output(IO_Y1+i,1);
		else
			drv_output(IO_Y1+i,0);
			
		tmp>>=1;
	}
}

//��ʱ�����������
void TimerEvent(void)
{
	UpdateOutput();
	LED_TOGGLE;
}

/*Ӧ���������ʼ������
�ڿ�ܳ���(frame.c)�е��õ�ΨһӦ�ýӿں���
������ҪΪ��ܳ���װ������Ҫ���¼�������*/
void InitApp(void)
{
	uint8_t i;
	
	//��eeprom ��ȡ�豸��ַ
	drv_eeprom_read(10,&i,1);
	if(i >= MB_BROADCAST_ADDR)//��Ч��ַ
	{
		i=g_HolReg[1]=1;//����ΪĬ�ϵ�ַ
		drv_eeprom_write(10,&i,1);
	}	
	
	//��ʼ���Ĵ���
	for(i=0;i<MODBUS_REG_SIZE;i++)
		g_HolReg[i]=0;
		
	frm_install_timer_event(0,500,TimerEvent);
	frm_install_event(EVENT_UART_ID,SerialEvent);//��װ���ڽ����¼�������
	
}
