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
	GPIOC->CR1|=(1<<3)|(1<<4)|(1<<5)|(1<<6);//ÍÆÍìÊä³ö
	GPIOC->CR2|=(1<<3)|(1<<4)|(1<<5)|(1<<6);//10MHz
	
	GPIOC->DDR&=~(1<<7);
	GPIOC->CR1|=(1<<7);
	GPIOC->CR2&=~(1<<7);
}

//spiµ¥×Ö½Ú¶ÁĞ´´¦Àí
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

//¶Á1702¼Ä´æÆ÷Î»
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

//Ğ´1702¼Ä´æÆ÷
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
	
	temp =Fm1702ReadReg(REG_CONTROL);						//Çå¿ÕFIFO
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
	if (temp >= 24)						//temp=255Ê±,»á½øÈëËÀÑ­»·
	{									//Òò´ËÔö¼ÓFIFO_LengthÔ½ÏŞÅĞ¶Ï
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
	
	Fm1702WriteReg(REG_COMMAND, cmd);					//ÃüÁîÖ´ĞĞ
	
	for(j = 0; j< RF_TRY_TIME; j++)				//¼ì²éÃüÁîÖ´ĞĞ·ñ
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


//¸Ãº¯ÊıÊµÏÖ±£´æ¿¨Æ¬ÊÕµ½µÄĞòÁĞºÅ
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

// ¸Ãº¯ÊıÉèÖÃ´ı·¢ËÍÊı¾İµÄ×Ö½ÚÊı
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

//¸´Î»²¢³õÊ¼»¯fm1702
uint8_t Fm1702Init(void)
{
	uint8_t i;
	uint16_t timeout;
 
	spi_init();
	DEV_RESET_SET;
	DelayMs(200);
	DEV_RESET_CLR;
	DelayMs(200);
	
	//µÈ´ıFM1702¸´Î»³É¹¦
	timeout=0;
	while(Fm1702ReadReg(REG_COMMAND)!=0)
	{
		DelayMs(1);
		if(++timeout > 1000)
			return 1;//¸´Î»Ê§°Ü
	}
	

	Fm1702WriteReg(REG_PAGE,0x80);                 //ÍùPAGÀïÃæĞ´0X80³õÊ¼»¯SPI½Ó¿Ú
	for(i=0;i<RF_TRY_TIME;i++)
	{
		if(Fm1702ReadReg(REG_COMMAND)==0)//Èç¹ûCommandÎª0±íÊ¾³õÊ¼»¯SPI½Ó¿Ú³É¹¦
			break;
	}
	if(i==RF_TRY_TIME)
		return 2;//Ê¹ÄÜspiÊ§°Ü

	Fm1702WriteReg(REG_PAGE,0);//¿ªÊ¼Ê¹ÓÃspi½Ó¿Ú
	
	/*²âÊÔ SPI½Ó¿ÚÊÇ·ñÕıÈ· 
	UartPutc(0xaa);
	UartPutc(Fm1702ReadReg(0x11)); //·µ»Ø0x58
	UartPutc(0xaa);*/
	
	////////// init_FM1702 ///////////////////////////////////////
	Fm1702WriteReg(REG_INT_EN,0x7f);	//address 06H  /* ÖĞ¶ÏÊ¹ÄÜ/½ûÖ¹¼Ä´æÆ÷ */off
	Fm1702WriteReg(REG_INT_REQ,0x7f);     //address 07H    /* ÖĞ¶ÏÇëÇó±êÊ¶¼Ä´æÆ÷ */Çå0
	Fm1702WriteReg(REG_TX_CONTROL,0x5b);	//address 11H    /* ·¢ËÍ¿ØÖÆ¼Ä´æÆ÷ */ 
	Fm1702WriteReg(REG_RX_CONTROL2,0x01);  //address 1EH    ÉèÖÃ½ÓÊÕ¿ØÖÆ¼Ä´æÆ÷
	Fm1702WriteReg(REG_RX_WAIT,0x07);		//address 21H    ÉèÖÃ½ÓÊÕºÍ·¢ËÍÖ®¼äµÄÊ±¼ä¼ä¸ô
	Fm1702WriteReg(REG_TYPESH,0x00);      //0:TYPEAÄ£Ê½*//* 1:TYPEBÄ£Ê½*//* 2:ÉÏº£Ä£Ê½*/
   
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
    Fm1702WriteReg(REG_BIT_FRAMEING,0x07);			//·¢ËÍ7bit
    Fm1702WriteReg(REG_CHANNEL_REDUNDANCY,0x03);	//¹Ø±ÕCRC
    Fm1702WriteReg(REG_TX_CONTROL,0x5b); 
    Fm1702WriteReg(REG_CONTROL,0x01); //temp;						//ÆÁ±ÎCRYPTO1Î»
		temp=RF_CMD_REQUEST_ALL;//ËùÓĞ¿¨
    temp = Fm1702Command(CMD_TRANSCEIVE,&temp,1);
		if(temp)
			return 1; //Ö´ĞĞÃüÁî´íÎó
	
    Fm1702ReadFIFO(g_Buffer);								//´ÓFIFOÖĞ¶ÁÈ¡Ó¦´ğĞÅÏ¢
    if((g_Buffer[0]!=0)&&(g_Buffer[1]==0))	//ÅĞ¶ÏÓ¦´ğĞÅºÅÊÇ·ñÕıÈ
		{
			g_CardType=g_Buffer[0];
			return 0;
		}

    return 2;
		
	/*	
	Fm1702WriteReg(0x11,0x58);//¹Ø·¢Éä¿ØÖÆ
	DelayMs(1);
	Fm1702WriteReg(0x11,0x5b);//¿ª·¢Éä¿ØÖÆ

	Fm1702WriteReg(0x0f,0x07);          //address 0FH  µ÷ÕûÎ»µÄÖ¡¸ñÊ½	
	temp = Fm1702ReadReg(0x09);
    temp&=0xf7;
	Fm1702WriteReg(0x09,temp);
	Fm1702WriteReg(0x22,0x03);
	temp=1;
	temp = Fm1702Command(CMD_TRANSCEIVE,&temp,1);
	if(temp)
		return 1;	
		
	DelayMs(1000);
	Fm1702ReadFIFO(g_Buffer);    //¶ÁÈ¡FIFOÀïÃæµÄÊı¾İ¼°Êı¾İ³¤¶È	
    if((g_Buffer[0]!=0)&&(g_Buffer[1]==0))						//ÅĞ¶ÏÓ¦´ğĞÅºÅÊÇ·ñÕıÈ·
        return 0;
	*/
	return 2;
}


// ·À³åÍ»¼ì²â
uint8_t PcdAntiColl(void)
{
	uint8_t	i,temp,row=0,col=0,pre_row=0;

	Fm1702WriteReg(0x23,0x63);//REG_CRC_PRESET_LSB
	Fm1702WriteReg(0x12,0x3f);//REG_CW_CONDUCTANCE
	Fm1702WriteReg(0x13,0x3f);//REG_MOD_CONDUCTANCE
	g_Buffer[0] = RF_CMD_ANTICOL;
	g_Buffer[1] = 0x20;
	Fm1702WriteReg(0x22,0x03);	        //¹Ø±ÕCRC,´ò¿ªÆæÅ¼Ğ£Ñé REG_CHANNEL_REDUNDANCY
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
		SaveUID(row, col, temp);			//½«ÊÕµ½µÄUID·ÅÈëUIDÊı×éÖĞ
		
		temp = Fm1702ReadReg(REG_ERROR_FLAG);				    // ÅĞ¶Ï½Ó…§Êı¾İÊÇ·ñ³ö´í 
		temp &= 0x01;
		if(temp == 0x00)
		{
			if(CheckUID())			//Ğ£ÑéÊÕµ½µÄUID
				return 3;
				
			return 0;
		}
		else
		{
			temp = Fm1702ReadReg(REG_COLLPOS);                 //¶ÁÈ¡³åÍ»¼ì²â¼Ä´æÆ÷
			row = temp / 8;
			col = temp % 8;
			g_Buffer[0] = RF_CMD_ANTICOL;
			SetBitFraming(row + pre_row, col);	//ÉèÖÃ´ı·¢ËÍÊı¾İµÄ×Ö½ÚÊı
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
/*Ãû³Æ: Select_Card */
/*¹¦ÄÜ: ¸Ãº¯ÊıÊµÏÖ¶Ô·ÅÈëFM1715²Ù×÷·¶Î§Ö®ÄÚµÄÄ³ÕÅ¿¨Æ¬½øĞĞÑ¡Ôñ*/
/*ÊäÈë: N/A */
/*Êä³ö: FM1715_NOTAGERR: ÎŞ¿¨*/
/* FM1715_PARITYERR: ÆæÅ¼Ğ£Ñé´í*/
/* FM1715_CRCERR: CRCĞ£Ñé´í*/
/* FM1715_BYTECOUNTERR: ½ÓÊÕ×Ö½Ú´íÎó*/
/* FM1715_OK: Ó¦´ğÕıÈ·*/
/* FM1715_SELERR: Ñ¡¿¨³ö´í*/
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

	Fm1702WriteReg(0x22,0x0f);	            // ¿ªÆôCRC,ÆæÅ¼Ğ£ÑéĞ£Ñé
	temp = Fm1702Command(CMD_TRANSCEIVE,g_Buffer,7);
	if(temp)
		return 1;//ÎŞ¿¨

	temp = Fm1702ReadReg(0x0A);
	if((temp & 0x02) == 0x02) return 2; //Ğ£Ñé´íÎó
	if((temp & 0x04) == 0x04) return 3; //(FM1715_FRAMINGERR);
	if((temp & 0x08) == 0x08) return 4; //crc´íÎó
	temp = Fm1702ReadReg(0x04);
	if(temp != 1) return 5;//½ÓÊÕ×Ö½Ú´íÎó
	Fm1702ReadFIFO(g_Buffer);	// ´ÓFIFOÖĞ¶ÁÈ¡Ó¦´ğĞÅÏ¢
	temp =g_Buffer[0];
	//ÅĞ¶ÏÓ¦´ğĞÅºÅÊÇ·ñÕıÈ·
	if((temp == 0x08) || (temp == 0x88) || (temp == 0x53) ||(temp == 0x18)) //S70 temp = 0x18	
		return 0;
	
	return 6;
}

//ÃÜÔ¿´Ófifo¼ÓÔØµ½FM1702ÃÜÔ¿»º³åÇø
uint8_t PcdLoadKey(uint8_t *keys)
{
	uint8_t i,lb,hb, temp;
	
	//ÃÜÔ¿¸ñÊ½×ª»»
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


//¹¦ÄÜ: ¸Ãº¯ÊıÊµÏÖÃÜÂëÈÏÖ¤µÄ¹ı³Ì
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
	Fm1702WriteReg(0x22,0x0f);	        // ¿ªÆôCRC,ÆæÅ¼Ğ£ÑéĞ£Ñé 
	
	temp =Fm1702Command(CMD_AUTHENT1,g_Buffer,6);// AUTHENT1
	if(temp)
		return 1;//±»¾Ü¾ø
	temp = Fm1702ReadReg(0x0A);
	if((temp & 0x02) == 0x02) return 2;//ÆæÅ¼Ğ£Ñé´íÎó
	if((temp & 0x04) == 0x04) return 3;//SOF´íÎó
	if((temp & 0x08) == 0x08) return 4;//CRC´íÎó
	
	temp = Fm1702Command(CMD_AUTHENT2,g_Buffer,0);	// AUTHENT2
	if(temp)
		return 5;//±»¾Ü¾ø
	temp = Fm1702ReadReg(0x0A);
	if((temp & 0x02) == 0x02) return 6;//ÆæÅ¼Ğ£Ñé´íÎó
	if((temp & 0x04) == 0x04) return 7;//SOF´íÎó
	if((temp & 0x08) == 0x08) return 8;//CRC´íÎó
	
	temp = Fm1702ReadReg(0x09);
	temp &= 0x08;	// Crypto1on=1ÑéÖ¤Í¨¹ı 
	if(temp == 0x08)
		return 0;

	return 9;//ÃÜÔ¿´íÎó£¬ÈÏÖ¤Ê§°Ü
}

//¶Á¿¨¡¡Ò»´Î¶ÁÈ¡Ò»¿é(16×Ö½Ú)  blockÎª¿é¾ø¶ÔµØÖ· Ö»ÄÜ¶ÁÈ¡µ±Ç°ÈÏÖ¤µÄÉÈÇøÖĞµÄ¿é 
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
		return 1; //±»¾Ü

	temp = Fm1702ReadReg(0x0A);
	if((temp & 0x02) == 0x02) return 2;
	if((temp & 0x04) == 0x04) return 3;
	if((temp & 0x08) == 0x08) return 4;
	temp = Fm1702ReadReg(0x04);
	if(temp == 0x10)	                      // 8K¿¨¶ÁÊı¾İ³¤¶ÈÎª16
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
		case 0x00:	return 3;//(FM1702_NOTAUTHERR);	     // ÔİÊ±ÆÁ±ÎµôĞ´´íÎó
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

//»ñÈ¡¿¨ºÅ
void GetUID(uint8_t *buff)
{
	uint8_t i;
	for(i=0;i<4;i++)
		buff[i]=g_CardUID[i];
}
