/*
	文件:App.c
	说明:CAFrame 应用层模板程序  -  功能:C727读卡器程序 
	
	芯艺设计室(http://www.chipart.cn)  版权所有
	工程主页:http://www.chipart.cn/projects/prj_hit.asp?id=8

	MCU :STM8S103F3
	时钟:12M 外部晶振
	编译:cosmic stm8 4.3.4 + stvd 4.3.10
	
  修改历史:
	2017-5-27   佟长福   DEMO
*/

#include "CAFrame.h"

#define NEW_SECTOR_INDEX 9
uint8_t g_DefKey[6]={0xff,0xff,0xff,0xff,0xff,0xff};
const uint8_t g_NewSectorNewKeyA[6]={0x8d,0xbf,0x08,0x9e,0xbb,0xa5};
const uint8_t g_NewSectorNewKeyB[6]={0x0e,0x55,0x35,0x34,0xaa,0x08};

#define OLD_SECTOR_INDEX 2
#define OLD_SECTOR_KEYS_COUNT 2
const uint8_t g_OldSectorKeys[OLD_SECTOR_KEYS_COUNT][6]={
	{0xb7,0xd1,0x81,0xe4,0xdf,0xe8},
	{0xb7,0xcf,0xec,0xec,0x5f,0x9d}
};
//const uint8_t g_OldSectorKeyA[6]={0xb7,0xd1,0x81,0xe4,0xdf,0xe8};
//const uint8_t g_OldSectorKeyB[6]={0x0c,0x9d,0xc5,0x9f,0xbb,0x29};
//const uint8_t g_OldSectorNewKeyA[6]={0xb8,0xd1,0x81,0xe4,0xdf,0xe8};
//const uint8_t g_OldSectorNewKeyB[6]={0x0d,0x9d,0xc5,0x9f,0xbb,0x29};

const uint8_t g_RWCode[4]={0x7D,0x27,0x88,0x32};//存取控制码
const uint8_t g_RWCodeDef[4]={0xff,0x07,0x80,0x69};//默认存取控制码
static uint8_t g_Buf[64]; //通用缓冲区
static uint8_t g_RecvBuf[SERIAL_BUFF_SIZE];//数据接收缓冲区

uint8_t g_LedCounter=0;
uint8_t g_BeepCounter=0;

void Beep(uint8_t n)
{
	BEEP_ON;
	g_BeepCounter=n;
	frm_timer_reset(0);
	frm_timer_enable(0);
}

void LedOn(uint8_t dly)
{
	LED_ON;
	g_LedCounter=dly;
	frm_timer_reset(0);
	frm_timer_enable(0);
}

uint16_t crc16_update(uint16_t crc, uint8_t a)
{
	int i;

	crc ^= a;
	for (i = 0; i < 8; ++i)
	{
		if (crc & 1)
				crc = (crc >> 1) ^ 0xA001;
		else
				crc = (crc >> 1);
	}
	return crc;
}
//CRC校验计算
uint16_t mbCRC16( uint8_t * pucFrame, uint16_t usLen )
{
	uint8_t hi,lo;
	uint16_t i;
	uint16_t crc;
	crc=0xFFFF;
	for (i=0;i<usLen;i++)
	{
		crc= crc16_update(crc, *pucFrame);
		pucFrame++;
	}
	hi=crc%256;
	lo=crc/256;
	crc=(hi<<8)|lo;
	
	return crc;
}
//解密余额
uint16_t ReadBalance(uint8_t *buf)
{
	uint8_t i,tmp;
	uint16_t val;
	
	if(mbCRC16(buf,10))
		return 0;
		
	val=0;
	for(i=0;i<8;i++)
	{
		val<<=2;
		tmp=~buf[i];
		tmp-=(i*9);
		if(tmp&0x02)
			val|=1;
		if(tmp&0x01)
			val|=2;
	}
	
	return val;
}
//加密余额
void WriteBalance(uint8_t *buf,uint16_t val)
{
	uint8_t i;
	uint8_t tmp;
	
	for(i=0;i<8;i++)
	{
		tmp=0;
		if(val&0x8000)
			tmp|=1;
		if(val&0x4000)
			tmp|=2;
			
		tmp+=(i*9);
		
		buf[i]=~tmp;
		val<<=2;
	}
	val=mbCRC16(buf,8);
	buf[8]=val>>8;
	buf[9]=val;
	buf[10]=0xff;
	buf[11]=0xff;
	buf[12]=0xff;
	buf[13]=0xff;
	buf[14]=0xff;
	buf[15]=0xff;
}

uint8_t GetCardID(void)
{
	uint8_t t;
	uint16_t crc;
	
	t=PcdRequest();
	if(t)
		return 1;

	t=PcdAntiColl();
	if(t)
		return 2;
		
	GetUID(g_RecvBuf+1);
	crc=mbCRC16(g_RecvBuf,5);
	g_RecvBuf[5]=crc>>8;
	g_RecvBuf[6]=crc;
	drv_serial_output(g_RecvBuf,7);
	
	return 0;
}

uint8_t ReadNewSector(void)
{
	uint8_t i,t;
	
	t=PcdRequest();
	if(t)
		return 1;

		t=PcdAntiColl();
	if(t)
		return 2;
		
	t=PcdSelectCard();
	if(t)
		return 3;
	
	t=PcdLoadKey(g_NewSectorNewKeyA);
	t=PcdAuthentication(NEW_SECTOR_INDEX,RF_CMD_AUTH_LA);
	if(t)
		return 4;
	
	for(i=0;i<3;i++)
	{
		t=CardRead(g_Buf+(i*16),NEW_SECTOR_INDEX*4+i);
		if(t)
			return 5;
	}
	drv_serial_output(g_Buf,48);
	return 0;	
}

uint8_t ReadOldSector(void)
{
	uint8_t i,t;
	
	for(i=0;i<OLD_SECTOR_KEYS_COUNT;i++)
	{
		t=PcdRequest();
		if(t)
			return 1;
	
			t=PcdAntiColl();
		if(t)
			return 2;
			
		t=PcdSelectCard();
		if(t)
			return 3;
	
		t=PcdLoadKey(g_OldSectorKeys[i]);
		t=PcdAuthentication(OLD_SECTOR_INDEX,RF_CMD_AUTH_LA);
		if(t==0)
			break;
	}
	if(i>=OLD_SECTOR_KEYS_COUNT)
		return 4;
	
	for(i=0;i<3;i++)
	{
		t=CardRead(g_Buf+(i*16),OLD_SECTOR_INDEX*4+i);
		if(t)
			return 5;
	}
	drv_serial_output(g_Buf,48);
	return 0;
}

//获取认证信息
uint8_t GetRegMessage(void)
{
	uint8_t i,t;
	uint16_t crc;
	
	for(i=0;i<OLD_SECTOR_KEYS_COUNT;i++)
	{
		t=PcdRequest();
		if(t)
			return 1;
	
			t=PcdAntiColl();
		if(t)
			return 2;
			
		t=PcdSelectCard();
		if(t)
			return 3;
	
		t=PcdLoadKey(g_OldSectorKeys[i]);
		t=PcdAuthentication(OLD_SECTOR_INDEX,RF_CMD_AUTH_LA);
		if(t==0)
			break;
	}
	if(i>=OLD_SECTOR_KEYS_COUNT)
		return 4;
	
	t=CardRead(g_Buf,OLD_SECTOR_INDEX*4);
	if(t)
		return 5;
		
	GetUID(g_RecvBuf+1);
	g_RecvBuf[5]=g_Buf[14];
	g_RecvBuf[6]=g_Buf[13];
	
	CardRead(g_Buf,OLD_SECTOR_INDEX*4+1); //读卡号
	g_RecvBuf[7]=g_Buf[2];
	g_RecvBuf[8]=g_Buf[1];
	
	crc=mbCRC16(g_RecvBuf,9);
	g_RecvBuf[9]=crc>>8;
	g_RecvBuf[10]=crc;
	drv_serial_output(g_RecvBuf,11);
	
	return 0;
}
//认证
uint8_t RegCard(void)
{
	uint8_t i,t;
	uint16_t crc;

	t=PcdRequest();
	if(t)
		return 1;

		t=PcdAntiColl();
	if(t)
		return 2;
		
	t=PcdSelectCard();
	if(t)
		return 3;
		
	//新扇区创建密码
	t=PcdLoadKey(g_DefKey);
	t=PcdAuthentication(NEW_SECTOR_INDEX,RF_CMD_AUTH_LA);
	if(t)
		return 4;
	
	t=0;
	for(i=0;i<6;i++,t++)
		g_Buf[i]=g_NewSectorNewKeyA[i];
	for(i=0;i<4;i++,t++)
		g_Buf[t]=g_RWCode[i];
	for(i=0;i<6;i++,t++)
		g_Buf[t]=g_NewSectorNewKeyB[i];
	
	t=CardWrite(g_Buf,NEW_SECTOR_INDEX*4+3);
	if(t)
		return t;
	
	//写入认证日期和余额
	t=PcdRequest();
	if(t)
		return 6;

	t=PcdAntiColl();
	if(t)
		return 7;
	
	//比对ID
	GetUID(g_Buf);
	for(i=0;i<4;i++)
	{
		if(g_Buf[i] != g_RecvBuf[9+i])
			break;
	}
	if(i<4)
		return 8;
		
	t=PcdSelectCard();
	if(t)
		return 9;
	t=PcdLoadKey(g_NewSectorNewKeyB);
	t=PcdAuthentication(NEW_SECTOR_INDEX,RF_CMD_AUTH_LB);
	if(t)
		return 10;
	
	//余额存储到扇区0块
	crc=g_RecvBuf[1];
	crc<<=8;
	crc+=g_RecvBuf[2];
	WriteBalance(g_Buf,crc);
	t=CardWrite(g_Buf,NEW_SECTOR_INDEX*4);
	if(t)
		return 11;
	
	//日期存储到第2个块
	memset(g_Buf,0,16);
	memcpy(g_Buf,g_RecvBuf+1,14);
	t=CardWrite(g_Buf,NEW_SECTOR_INDEX*4+2);
	if(t)
		return 12;
	
	//返回命令码表示成功
	g_Buf[0]=0xA3;
	crc=mbCRC16(g_Buf,1);
	g_Buf[1]=crc>>8;
	g_Buf[2]=crc;
	drv_serial_output(g_Buf,3);
	
	return 0;
}

uint8_t RestoreCard(void)
{
	uint8_t i,t;
	uint16_t crc;

	t=PcdRequest();
	if(t)
		return 1;

		t=PcdAntiColl();
	if(t)
		return 2;
		
	t=PcdSelectCard();
	if(t)
		return 3;
		
	//反认证,恢复
	t=PcdLoadKey(g_NewSectorNewKeyB);
	t=PcdAuthentication(NEW_SECTOR_INDEX,RF_CMD_AUTH_LB);
	if(t)
		return 4;
	
	t=0;
	for(i=0;i<6;i++,t++)
		g_Buf[i]=g_DefKey[i];
	for(i=0;i<4;i++,t++)
		g_Buf[t]=g_RWCodeDef[i];
	for(i=0;i<6;i++,t++)
		g_Buf[t]=g_DefKey[i];
	
	t=CardWrite(g_Buf,NEW_SECTOR_INDEX*4+3);
	if(t)
		return 5;
	
	//返回命令码表示成功	
	g_Buf[0]=0XA4;
	crc=mbCRC16(g_Buf,1);
	g_Buf[1]=crc>>8;
	g_Buf[2]=crc;
	drv_serial_output(g_Buf,3);
	return 0;
}

void SendErrorCode(uint8_t err)
{
	uint16_t crc;
	
	g_Buf[0]=0xe0;
	g_Buf[1]=err;
	crc = mbCRC16(g_Buf,2);
	g_Buf[2]=crc>>8;
	g_Buf[3]=crc;
	drv_serial_output(g_Buf,4);
}

uint8_t GetBalance(void)
{
	uint8_t t;
	uint16_t val;
	
	t=PcdRequest();
	if(t)
		return 1;

	t=PcdAntiColl();
	if(t)
		return 2;
		
	t=PcdSelectCard();
	if(t)
		return 3;

#if 1
	//读加密后
	t=PcdLoadKey(g_NewSectorNewKeyA);
	t=PcdAuthentication(NEW_SECTOR_INDEX,RF_CMD_AUTH_LA);
	if(t)
		return 4;
#else
	//读未加密时
	t=PcdLoadKey(g_DefKey);
	t=PcdAuthentication(NEW_SECTOR_INDEX,RF_CMD_AUTH_LA);
	if(t)
		return 4;
#endif
	
	t=CardRead(g_Buf,NEW_SECTOR_INDEX*4);
	if(t)
		return 5;
	
	val=ReadBalance(g_Buf);
	g_Buf[0]=0xa0;
	g_Buf[1]=val>>8;
	g_Buf[2]=val;
	val=mbCRC16(g_Buf,3);
	g_Buf[3]=val>>8;
	g_Buf[4]=val;
	
	drv_serial_output(g_Buf,5);
	
	return 0;
}

void UartRecv(void)
{
	uint8_t len,err;
	len=drv_serial_input(g_RecvBuf);
	if(len<3)
		return ;
	if(mbCRC16(g_RecvBuf,len)!=0)
		return ;

	if(g_RecvBuf[0]==0xA0)				//读余额
	{
		err=GetBalance();
		if(err)
			SendErrorCode(err);
	}
	else if(g_RecvBuf[0]==0xA1)		//获取卡号(查询记录用)
	{
		err=GetCardID();
		if(err)
			SendErrorCode(err);
	}
	else if(g_RecvBuf[0]==0xA2)		//认证资料获取
	{
		err=GetRegMessage();
		if(err)
			SendErrorCode(err);
	}
	else if(g_RecvBuf[0]==0xA3)		//认证卡
	{
		err=RegCard();
		if(err)
			SendErrorCode(err);
	}
	else if(g_RecvBuf[0]==0xA4)		//恢复
	{
		//测试期间使用
		err=RestoreCard();
		if(err)
			SendErrorCode(err);
	}
	else if(g_RecvBuf[0]==0xA5) //读旧扇区数据
	{
		err=ReadOldSector();
	}
	else if(g_RecvBuf[0]=0xA6) //读新扇区数据
	{
		err=ReadNewSector();
	}
	else
		return ;
		
	if(err)
	{
		LedOn(4);
		Beep(6);
	}
	else
	{
		LedOn(2);
		Beep(1);
	}
}

//定时更新输入输出
void Timer1Event(void)
{
	if((g_LedCounter==0)&&(g_BeepCounter==0))
	{
		LED_OFF;
		BEEP_OFF;
		frm_timer_disable(0);
		return ;
	}
	
	if(g_LedCounter)
		g_LedCounter--;
	else
		LED_OFF;
		
	if(g_BeepCounter)
	{
		if(g_BeepCounter%2)
			BEEP_ON;
		else
			BEEP_OFF;
		g_BeepCounter--;
	}
	else
		BEEP_OFF;
}

/*应用主程序初始化函数
在框架程序(frame.c)中调用的唯一应用接口函数
在这里要为框架程序安装所有需要的事件处理函数*/
void InitApp(void)
{
	uint8_t t;
	
	LED_POWER_ON;
	LED_OFF;
		
	frm_install_timer_event(0,50,Timer1Event);	//蜂鸣器、LED控制定时器
	frm_install_event(EVENT_UART_ID,UartRecv);
	
	Fm1702Init();
	
	frm_timer_disable(0);
}

