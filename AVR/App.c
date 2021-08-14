/****************************************************************************|
	文件:App.c
	CAFrame 应用层模板程序
	
	芯艺设计室(http://www.chipart.cn)  版权所有
	工程主页:http://www.chipart.cn/projects/prj_hit.asp?id=8

	MCU :ATMEGA328
	时钟:外部11.0592MHz晶振
	编译:WinAVR20100110

	修改记录:
		2019-04-27 　创建

*****************************************************************************/

#include "CAFrame.h"
#include <util/crc16.h>


/*---------------------------------------------------------------------------
命令码03h/10h MODBUS地址 	
											0:输出状态寄存器g_HolReg[0]
											1:设备地址　g_HolReg[1]
											2~9:保留

---------------------------------------------------------------------------*/
#define MB_BROADCAST_ADDR 0xFA  //广播地址


#define MODBUS_REG_SIZE 10
uint16_t g_HolReg[MODBUS_REG_SIZE];  //MODBUS寄存器 
uint8_t g_mbBuf[SERIAL_BUFF_SIZE];    //通信用缓冲区


//CRC校验计算
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

//MODBUS 命令处理 函数
void mbCommand(uint8_t *buf,uint8_t len)
{
	uint8_t i,size,tmp;
	uint16_t addr,crc;
	
	i=g_HolReg[1];
	if((buf[0] != MB_BROADCAST_ADDR)&& (buf[0] != i))	//地址不符
		return ;	
	
	if(mbCRC16(buf,len)) //校验
		return ;	
		
	addr=BufToReg(buf+2);		
	size=BufToReg(buf+4);	//忽略高字节	
	if(buf[1]==0x03) 			//读多个寄存器 功能码03h
	{
		if(addr+size > MODBUS_REG_SIZE)  	//无效地址和长度
			return ;
		buf[2]=size*2; 			//数据长度
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
	else if(buf[1]==0x10) //写多个寄存器 功能码10h
	{
		if(addr+size>4)			//数据地址和长度不符
			return ;
		if(buf[6]!=size*2)	//写入长度不符
			return ;
		for(i=0;i<size;i++)
			g_HolReg[addr+i]=BufToReg(buf+7+(i*2));
		crc=mbCRC16(buf,6);
		RegToBuf(buf+6,crc);
		drv_serial_output(buf,8);
	}
	else if(buf[1]==0x01)//读线圈 功能码01h
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
	else if(buf[1]==0x05)//设置线圈:功能码05h
	{
		if((addr<1)||(addr>8))
			return ;
		tmp=addr-1;
		
		if(size == 0xaa) 
			g_HolReg[0]|=1<<tmp;
		else
			g_HolReg[0]&=~(1<<tmp);
			
		drv_serial_output(buf,len);//命令原样返回	
	}
	else;
}

//串口接收事件
void SerialEvent(void)
{
    uint8_t size = drv_serial_input(g_mbBuf);
    mbCommand(g_mbBuf,size); 
    //drv_serial_output(g_mbBuf,size); //测试用
}

//定时更新输出状态
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

//定时更新输入输出
void TimerEvent(void)
{
	UpdateOutput();
	LED_TOGGLE;
}

/*应用主程序初始化函数
在框架程序(frame.c)中调用的唯一应用接口函数
在这里要为框架程序安装所有需要的事件处理函数*/
void InitApp(void)
{
	uint8_t i;
	
	//从eeprom 读取设备地址
	drv_eeprom_read(10,&i,1);
	if(i >= MB_BROADCAST_ADDR)//无效地址
	{
		i=g_HolReg[1]=1;//设置为默认地址
		drv_eeprom_write(10,&i,1);
	}	
	
	//初始化寄存器
	for(i=0;i<MODBUS_REG_SIZE;i++)
		g_HolReg[i]=0;
		
	frm_install_timer_event(0,500,TimerEvent);
	frm_install_event(EVENT_UART_ID,SerialEvent);//安装串口接收事件处理函数
	
}
