//fm1702.c

#include "caframe.h"

#define RF_TRY_TIME 128

static uint8_t g_Buffer[64];
static uint8_t g_CardUID[5];
static uint8_t g_CardType;

#define DEV_RESET_SET	GPIOC->ODR|=(1<<3)
#define DEV_RESET_CLR GPIOC->ODR&=~(1<<3)
#define DEV_SELECT GPIOC->ODR&=~(1<<4)
#define DEV_UNSELECT GPIOC->ODR|=(1<<4)
#define SPI_SCK_SET GPIOC->ODR|=(1<<5)
#define SPI_SCK_CLR	GPIOC->ODR&=~(1<<5)
#define SPI_MOSI_SET GPIOC->ODR|=(1<<6)
#define SPI_MOSI_CLR GPIOC->ODR&=~(1<<6)
#define SPI_MISO_STATUS	(GPIOC->IDR & (1<<7))

static void spi_init(void)
{
	DEV_UNSELECT;
	SPI_SCK_CLR;
	//NSS,SCK,MOSI
	GPIOC->DDR|=(1<<3)|(1<<4)|(1<<5)|(1<<6);
	GPIOC->CR1|=(1<<3)|(1<<4)|(1<<5)|(1<<6);//推挽输出
	GPIOC->CR2|=(1<<3)|(1<<4)|(1<<5)|(1<<6);//10MHz
	
	GPIOC->DDR&=~(1<<7);
	GPIOC->CR1|=(1<<7);
	GPIOC->CR2&=~(1<<7);
}

//spi单字节读写处理
static uint8_t spi_rw_byte(uint8_t dat)
{
	uint8_t i,ret=0;
	for(i=0;i<8;i++)
	{
		if(dat&0x80)
			SPI_MOSI_SET;
		else
			SPI_MOSI_CLR;
			
		SPI_SCK_SET;
		
		ret<<=1;
		if(SPI_MISO_STATUS)
			ret|=1;
			
		SPI_SCK_CLR;
		
		dat<<=1;
	}
	return ret;
}

//读1702寄存器位
static uint8_t Fm1702ReadReg(uint8_t addr)
{
	uint8_t Result=0;
	
	DEV_SELECT;
	addr =(addr<<1)|0x80;
	
	spi_rw_byte(addr);
	Result=spi_rw_byte(0);
	
	DEV_UNSELECT;
	
	return Result;
}

//写1702寄存器
static void Fm1702WriteReg(uint8_t addr, uint8_t value)
{  
    DEV_SELECT;
    addr = ((addr<<1)&0x7E);
		spi_rw_byte(addr);
    spi_rw_byte(value);
    DEV_UNSELECT;
}

static void Fm1702ClearFIFO(void)
{
	uint8_t temp;
	
	temp =Fm1702ReadReg(REG_CONTROL);						//清空FIFO
	temp = (temp | 0x01);
	Fm1702WriteReg(REG_CONTROL, temp);
}

static void Fm1702WriteFIFO(uint8_t *buff,uint8_t len)
{
	uint8_t i;
	
	for(i = 0; i < len; i++)
		Fm1702WriteReg(REG_FIFO,buff[i]);
}

static uint8_t Fm1702ReadFIFO(uint8_t *buff)
{
	uint8_t temp;
	uint8_t i;
	
	temp =Fm1702ReadReg(REG_FIFO_LENGTH);
	if (temp == 0)
	{
		return 0;
	}
	if (temp >= 24)						//temp=255时,会进入死循环
	{									//因此增加FIFO_Length越限判断
		temp = 24;						//yanshouli,2003-12-2
	}
	for(i = 0;i < temp; i++)
	{
 		buff[i] =Fm1702ReadReg(REG_FIFO);
	}
	return temp;
 }


static uint8_t Fm1702Command(uint8_t cmd,uint8_t *buff,uint8_t len)
{
	uint16_t  j;
	uint8_t temp;
	
	Fm1702WriteReg(REG_COMMAND, 0x00);
	Fm1702ClearFIFO();
	if(len>0)
		Fm1702WriteFIFO(buff,len);

	temp =Fm1702ReadReg(REG_FIFO_LENGTH);
	
	Fm1702WriteReg(REG_COMMAND, cmd);					//命令执行
	
	for(j = 0; j< RF_TRY_TIME; j++)				//检查命令执行否
	{
		DelayMs(1);
		temp =Fm1702ReadReg(REG_COMMAND);
		if(temp == 0x00)  
			return 0;
	}
	return 1;	
}

uint8_t CheckUID(void)
{
	uint8_t i,temp;

	temp = 0x00;
	for(i = 0; i < 5; i++)
		temp = temp ^ g_CardUID[i];

	if(temp == 0)
		return 0;

	return 1;
}


//该函数实现保存卡片收到的序列号
static void SaveUID(uint8_t row, uint8_t col, uint8_t length)
{
	uint8_t	i,temp,temp1;

	if((row == 0x00) && (col == 0x00))
	{
		for(i = 0; i < length; i++)
			g_CardUID[i] = g_Buffer[i];
	}
	else
	{
		temp = g_Buffer[0];
		temp1 =g_CardUID[row - 1];
		switch(col)
		{
		case 0:		temp1 = 0x00; row = row + 1; break;
		case 1:		temp = temp & 0xFE; temp1 = temp1 & 0x01; break;
		case 2:		temp = temp & 0xFC; temp1 = temp1 & 0x03; break;
		case 3:		temp = temp & 0xF8; temp1 = temp1 & 0x07; break;
		case 4:		temp = temp & 0xF0; temp1 = temp1 & 0x0F; break;
		case 5:		temp = temp & 0xE0; temp1 = temp1 & 0x1F; break;
		case 6:		temp = temp & 0xC0; temp1 = temp1 & 0x3F; break;
		case 7:		temp = temp & 0x80; temp1 = temp1 & 0x7F; break;
		default:	break;
		}

		g_Buffer[0] = temp;
		g_CardUID[row - 1] = temp1 | temp;
		for(i = 1; i < length; i++)
			g_CardUID[row - 1 + i] = g_Buffer[i];
	}
}

// 该函数设置待发送数据的字节数
static void SetBitFraming(uint8_t row, uint8_t col)
{
	switch(row)
	{
	case 0:		g_Buffer[1] = 0x20; break;
	case 1:		g_Buffer[1] = 0x30; break;
	case 2:		g_Buffer[1] = 0x40; break;
	case 3:		g_Buffer[1] = 0x50; break;
	case 4:		g_Buffer[1] = 0x60; break;
	default:	break;
	}

	switch(col)
	{
	case 0:		Fm1702WriteReg(0x0F,0x00);  break;
	case 1:		Fm1702WriteReg(0x0F,0x11); g_Buffer[1] |= 0x01; break;
	case 2:		Fm1702WriteReg(0x0F,0x22); g_Buffer[1] |= 0x02; break;
	case 3:		Fm1702WriteReg(0x0F,0x33); g_Buffer[1] |= 0x03; break;
	case 4:		Fm1702WriteReg(0x0F,0x44); g_Buffer[1] |= 0x04; break;
	case 5:		Fm1702WriteReg(0x0F,0x55); g_Buffer[1] |= 0x05; break;
	case 6:		Fm1702WriteReg(0x0F,0x66); g_Buffer[1] |= 0x06; break;
	case 7:		Fm1702WriteReg(0x0F,0x77); g_Buffer[1] |= 0x07; break;
	default:	break;
	}
}

//复位并初始化fm1702
uint8_t Fm1702Init(void)
{
	uint8_t i;
	uint16_t timeout;
 
	spi_init();
	DEV_RESET_SET;
	DelayMs(200);
	DEV_RESET_CLR;
	DelayMs(200);
	
	//等待FM1702复位成功
	timeout=0;
	while(Fm1702ReadReg(REG_COMMAND)!=0)
	{
		DelayMs(1);
		if(++timeout > 1000)
			return 1;//复位失败
	}
	

	Fm1702WriteReg(REG_PAGE,0x80);                 //往PAG里面写0X80初始化SPI接口
	for(i=0;i<RF_TRY_TIME;i++)
	{
		if(Fm1702ReadReg(REG_COMMAND)==0)//如果Command为0表示初始化SPI接口成功
			break;
	}
	if(i==RF_TRY_TIME)
		return 2;//使能spi失败

	Fm1702WriteReg(REG_PAGE,0);//开始使用spi接口
	
	/*测试 SPI接口是否正确 
	UartPutc(0xaa);
	UartPutc(Fm1702ReadReg(0x11)); //返回0x58
	UartPutc(0xaa);*/
	
	////////// init_FM1702 ///////////////////////////////////////
	Fm1702WriteReg(REG_INT_EN,0x7f);	//address 06H  /* 中断使能/禁止寄存器 */off
	Fm1702WriteReg(REG_INT_REQ,0x7f);     //address 07H    /* 中断请求标识寄存器 */清0
	Fm1702WriteReg(REG_TX_CONTROL,0x5b);	//address 11H    /* 发送控制寄存器 */ 
	Fm1702WriteReg(REG_RX_CONTROL2,0x01);  //address 1EH    设置接收控制寄存器
	Fm1702WriteReg(REG_RX_WAIT,0x07);		//address 21H    设置接收和发送之间的时间间隔
	Fm1702WriteReg(REG_TYPESH,0x00);      //0:TYPEA模式*//* 1:TYPEB模式*//* 2:上海模式*/
   
   return 0;
}


uint8_t PcdRequest(void)
{
    uint8_t  temp;

    Fm1702WriteReg(REG_TX_CONTROL,0x58);
    ;//delay1(1);
    Fm1702WriteReg(REG_TX_CONTROL,0x5b);		 
    Fm1702WriteReg(REG_CRC_PRESET_LSB,0x63);
    Fm1702WriteReg(REG_CW_CONDUCTANCE,0x3f);
    Fm1702WriteReg(REG_BIT_FRAMEING,0x07);			//发送7bit
    Fm1702WriteReg(REG_CHANNEL_REDUNDANCY,0x03);	//关闭CRC
    Fm1702WriteReg(REG_TX_CONTROL,0x5b); 
    Fm1702WriteReg(REG_CONTROL,0x01); //temp;						//屏蔽CRYPTO1位
		temp=RF_CMD_REQUEST_ALL;//所有卡
    temp = Fm1702Command(CMD_TRANSCEIVE,&temp,1);
		if(temp)
			return 1; //执行命令错误
	
    Fm1702ReadFIFO(g_Buffer);								//从FIFO中读取应答信息
    if((g_Buffer[0]!=0)&&(g_Buffer[1]==0))	//判断应答信号是否正�
		{
			g_CardType=g_Buffer[0];
			return 0;
		}

    return 2;
		
	/*	
	Fm1702WriteReg(0x11,0x58);//关发射控制
	DelayMs(1);
	Fm1702WriteReg(0x11,0x5b);//开发射控制

	Fm1702WriteReg(0x0f,0x07);          //address 0FH  调整位的帧格式	
	temp = Fm1702ReadReg(0x09);
    temp&=0xf7;
	Fm1702WriteReg(0x09,temp);
	Fm1702WriteReg(0x22,0x03);
	temp=1;
	temp = Fm1702Command(CMD_TRANSCEIVE,&temp,1);
	if(temp)
		return 1;	
		
	DelayMs(1000);
	Fm1702ReadFIFO(g_Buffer);    //读取FIFO里面的数据及数据长度	
    if((g_Buffer[0]!=0)&&(g_Buffer[1]==0))						//判断应答信号是否正确
        return 0;
	*/
	return 2;
}


// 防冲突检测
uint8_t PcdAntiColl(void)
{
	uint8_t	i,temp,row=0,col=0,pre_row=0;

	Fm1702WriteReg(0x23,0x63);//REG_CRC_PRESET_LSB
	Fm1702WriteReg(0x12,0x3f);//REG_CW_CONDUCTANCE
	Fm1702WriteReg(0x13,0x3f);//REG_MOD_CONDUCTANCE
	g_Buffer[0] = RF_CMD_ANTICOL;
	g_Buffer[1] = 0x20;
	Fm1702WriteReg(0x22,0x03);	        //关闭CRC,打开奇偶校验 REG_CHANNEL_REDUNDANCY
	temp = Fm1702Command(CMD_TRANSCEIVE,g_Buffer,2);
	while(1)
	{
		if(temp)
			return 1;

		temp = Fm1702ReadReg(REG_FIFO_LENGTH);
		if((temp == 0)||(temp>5))
			return 2;
			
		//UartPutc(temp);

		Fm1702ReadFIFO(g_Buffer);
		SaveUID(row, col, temp);			//将收到的UID放入UID数组中
		
		temp = Fm1702ReadReg(REG_ERROR_FLAG);				    // 判断接収数据是否出错 
		temp &= 0x01;
		if(temp == 0x00)
		{
			if(CheckUID())			//校验收到的UID
				return 3;
				
			return 0;
		}
		else
		{
			temp = Fm1702ReadReg(REG_COLLPOS);                 //读取冲突检测寄存器
			row = temp / 8;
			col = temp % 8;
			g_Buffer[0] = RF_CMD_ANTICOL;
			SetBitFraming(row + pre_row, col);	//设置待发送数据的字节数
			pre_row = pre_row + row;
			for(i = 0; i < pre_row + 1; i++)
				g_Buffer[i + 2] = g_CardUID[i];

			if(col != 0x00)
				row = pre_row + 1;
			else
				row = pre_row;

			temp = Fm1702Command(CMD_TRANSCEIVE,g_Buffer,row + 2);
		}
	}
}

/****************************************************************/
/*名称: Select_Card */
/*功能: 该函数实现对放入FM1715操作范围之内的某张卡片进行选择*/
/*输入: N/A */
/*输出: FM1715_NOTAGERR: 无卡*/
/* FM1715_PARITYERR: 奇偶校验错*/
/* FM1715_CRCERR: CRC校验错*/
/* FM1715_BYTECOUNTERR: 接收字节错误*/
/* FM1715_OK: 应答正确*/
/* FM1715_SELERR: 选卡出错*/
/****************************************************************/
uint8_t PcdSelectCard(void)
{
	uint8_t  i,temp;

	Fm1702WriteReg(0x23,0x63);
	Fm1702WriteReg(0x12,0x3f);
	g_Buffer[0] = RF_CMD_SELECT;
	g_Buffer[1] = 0x70;
	for(i = 0; i < 5; i++)
		g_Buffer[i + 2] = g_CardUID[i];

	Fm1702WriteReg(0x22,0x0f);	            // 开启CRC,奇偶校验校验
	temp = Fm1702Command(CMD_TRANSCEIVE,g_Buffer,7);
	if(temp)
		return 1;//无卡

	temp = Fm1702ReadReg(0x0A);
	if((temp & 0x02) == 0x02) return 2; //校验错误
	if((temp & 0x04) == 0x04) return 3; //(FM1715_FRAMINGERR);
	if((temp & 0x08) == 0x08) return 4; //crc错误
	temp = Fm1702ReadReg(0x04);
	if(temp != 1) return 5;//接收字节错误
	Fm1702ReadFIFO(g_Buffer);	// 从FIFO中读取应答信息
	temp =g_Buffer[0];
	//判断应答信号是否正确
	if((temp == 0x08) || (temp == 0x88) || (temp == 0x53) ||(temp == 0x18)) //S70 temp = 0x18	
		return 0;
	
	return 6;
}

//密钥从fifo加载到FM1702密钥缓冲区
uint8_t PcdLoadKey(uint8_t *keys)
{
	uint8_t i,lb,hb, temp;
	
	//密钥格式转换
	for(i=0;i<6;i++)
	{
		lb=keys[i]&0x0f;
		hb=keys[i]>>4;
		g_Buffer[i*2]=(~hb << 4) | hb;
		g_Buffer[i*2+1]=(~lb << 4) | lb;
	}
	
	temp = Fm1702Command(CMD_LOADKEY,g_Buffer,12);
	if(temp)
		return 1;
	temp = Fm1702ReadReg(0x0A) & 0x40;
	if (temp == 0x40)
		return 2;
	
	return 0;
}


//功能: 该函数实现密码认证的过程
uint8_t PcdAuthentication(uint8_t sec, uint8_t mode)
{
	uint8_t i;
	uint8_t temp;

	Fm1702WriteReg(0x23,0x63);
	Fm1702WriteReg(0x12,0x3f);
	Fm1702WriteReg(0x13,0x3f);
	temp = Fm1702ReadReg(0x09);
	temp &= 0xf7;
	Fm1702WriteReg(0x09,temp);
	if(mode == RF_CMD_AUTH_LB)			
		g_Buffer[0] = RF_CMD_AUTH_LB;
	else
		g_Buffer[0] = RF_CMD_AUTH_LA;
	g_Buffer[1] = sec * 4 + 3;
	for(i = 0; i < 4; i++)
		g_Buffer[2 + i] = g_CardUID[i];
	Fm1702WriteReg(0x22,0x0f);	        // 开启CRC,奇偶校验校验 
	
	temp =Fm1702Command(CMD_AUTHENT1,g_Buffer,6);// AUTHENT1
	if(temp)
		return 1;//被拒绝
	temp = Fm1702ReadReg(0x0A);
	if((temp & 0x02) == 0x02) return 2;//奇偶校验错误
	if((temp & 0x04) == 0x04) return 3;//SOF错误
	if((temp & 0x08) == 0x08) return 4;//CRC错误
	
	temp = Fm1702Command(CMD_AUTHENT2,g_Buffer,0);	// AUTHENT2
	if(temp)
		return 5;//被拒绝
	temp = Fm1702ReadReg(0x0A);
	if((temp & 0x02) == 0x02) return 6;//奇偶校验错误
	if((temp & 0x04) == 0x04) return 7;//SOF错误
	if((temp & 0x08) == 0x08) return 8;//CRC错误
	
	temp = Fm1702ReadReg(0x09);
	temp &= 0x08;	// Crypto1on=1验证通过 
	if(temp == 0x08)
		return 0;

	return 9;//密钥错误，认证失败
}

//读卡　一次读取一块(16字节)  block为块绝对地址 只能读取当前认证的扇区中的块 
uint8_t CardRead(uint8_t *buff, uint8_t block)
{
	uint8_t	temp;

	Fm1702WriteReg(0x23,0x63);
	Fm1702WriteReg(0x12,0x3f);
	Fm1702WriteReg(0x13,0x3f);
	Fm1702WriteReg(0x22,0x0f);
	
	buff[0] = RF_CMD_READ;
	buff[1] = block;
	if(Fm1702Command(CMD_TRANSCEIVE,buff,2))
		return 1; //被拒

	temp = Fm1702ReadReg(0x0A);
	if((temp & 0x02) == 0x02) return 2;
	if((temp & 0x04) == 0x04) return 3;
	if((temp & 0x08) == 0x08) return 4;
	temp = Fm1702ReadReg(0x04);
	if(temp == 0x10)	                      // 8K卡读数据长度为16
	{
		Fm1702ReadFIFO(buff);
		return 0;
	}
	else
	{
		//UartPutc(temp);
		return 5;
	}
}

uint8_t CardWrite(uint8_t *buff, uint8_t block)
{
	uint8_t	temp;

	Fm1702WriteReg(0x23,0x63);
	Fm1702WriteReg(0x12,0x3f);
//	F_buff = temp1 + 0x10;
	Fm1702WriteReg(0x22,0x07);
	
	g_Buffer[0] = RF_CMD_WRITE;
	g_Buffer[1] = block;
	temp = Fm1702Command(CMD_TRANSCEIVE,g_Buffer,2);
	if(temp)
		return 1;

	temp = Fm1702ReadReg(0x04);
	if(temp == 0)
		return 2;

	Fm1702ReadFIFO(g_Buffer);
	temp = g_Buffer[0];
	switch(temp)
	{
		case 0x00:	return 3;//(FM1702_NOTAUTHERR);	     // 暂时屏蔽掉写错误
		case 0x04:	return 4;//(FM1702_EMPTY);
		case 0x0a:	break;
		case 0x01:	return 5;//(FM1702_CRCERR);
		case 0x05:	return 6;//(FM1702_PARITYERR);
		default:	return	7;//(FM1702_WRITEERR);
	}

	temp = Fm1702Command(CMD_TRANSCEIVE,buff, 16);
	
	if(temp==0)
		return 0;

	temp = Fm1702ReadReg(0x0A);
	if((temp & 0x02) == 0x02)
		return 8;//(FM1702_PARITYERR);
	else if((temp & 0x04) == 0x04)
		return 9;//(FM1702_FRAMINGERR);
	else if((temp & 0x08) == 0x08)
		return 10;//(FM1702_CRCERR);
	else
		return 11;//(FM1702_WRITEERR);
}

//获取卡号
void GetUID(uint8_t *buff)
{
	uint8_t i;
	for(i=0;i<4;i++)
		buff[i]=g_CardUID[i];
}
