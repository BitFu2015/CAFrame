//FM1702.H
#ifndef FM1702_H
#define FM1702_H

//接口函数
uint8_t Fm1702Init(void);   
uint8_t PcdRequest(void);
uint8_t PcdAntiColl(void);
uint8_t PcdLoadKey(uint8_t *keys);
uint8_t PcdSelectCard(void);
uint8_t PcdAuthentication(uint8_t sec, uint8_t mode);
uint8_t CardRead(uint8_t *buff, uint8_t Block_Adr);
uint8_t CardWrite(uint8_t *buff, uint8_t block);
void GetUID(uint8_t *buff);

//1702命令字
#define CMD_IDLE              0x00               //取消当前命令
#define CMD_WRITEE2           0x01               //写EEPROM
#define CMD_READE2            0x03               //读EEPROM
#define CMD_LOADCONFIG        0x07               //调EEPROM中保存的寄存器设置
#define CMD_LOADKEYE2         0x0B               //将EEPROM中保存的密钥调入缓存
#define CMD_AUTHENT1          0x0C               //验证密钥第一步
#define CMD_AUTHENT2          0x14               //验证密钥第二步
#define	CMD_RECEIVE           0x16               //接收数据
#define CMD_LOADKEY           0x19               //传送密钥
#define CMD_TRANSMIT          0x1A               //发送数据
#define CMD_TRANSCEIVE        0x1E               //发送并接收数据
#define CMD_RESETPHASE        0x3F               //复位
#define CMD_CALCCRC           0x12               //CRC计算

//射频卡通信命令码定义
#define RF_CMD_REQUEST_STD	0x26
#define RF_CMD_REQUEST_ALL	0x52
#define RF_CMD_ANTICOL		0x93
#define RF_CMD_SELECT		0x93
#define RF_CMD_AUTH_LA		0x60
#define RF_CMD_AUTH_LB		0x61
#define RF_CMD_READ		    0x30
#define RF_CMD_WRITE		0xa0
#define RF_CMD_INC		    0xc1
#define RF_CMD_DEC		    0xc0
#define RF_CMD_RESTORE		0xc2
#define RF_CMD_TRANSFER		0xb0
#define RF_CMD_HALT		    0x50


//1702 FIFO长度定义
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte


//1702寄存器定义
// PAGE 0
#define     REG_PAGE               0x00    
#define     REG_COMMAND            0x01    
#define     REG_FIFO          		 0x02    
#define     REG_PRIMARY_STATUS     0x03    
#define     REG_FIFO_LENGTH        0x04    
#define     REG_SECONDRY_STATUS    0x05
#define     REG_INT_EN       0x06    
#define     REG_INT_REQ      0x07    
// PAGE 1      Control and Status
#define     REG_CONTROL           0x09    
#define     REG_ERROR_FLAG        0x0A
#define     REG_COLLPOS           0x0B
#define     REG_TIMER_VALUE         0x0C
#define     REG_CRC_RESULT_LSB     0x0D
#define     REG_CRC_RESULT_MSB     0x0E
#define     REG_BIT_FRAMEING         0x0F
// PAGE 2      Transmitter and Coder Control
#define     REG_TX_CONTROL          0x11
#define     REG_CW_CONDUCTANCE      0x12
#define     REG_MOD_CONDUCTANCE     0x13
#define     REG_CODER_CONTROL       0x14
#define     REG_MOD_WIDTH           0x15
#define     REG_MOD_WIDTH_SOF        0x16
#define     REG_TYPEB_FRAMING       0x17
// PAGE 3      Receiver and Decoder Control
#define     REG_RX_CONTROL1         0x19
#define     REG_DECODER_CONTROL     0x1A
#define     REG_BIT_PHASE           0x1B
#define     REG_RX_THREASHOLD        0x1C
#define     REG_BPSKDEM_CONTROL     0x1D
#define     REG_RX_CONTROL2         0x1E
#define     REG_CLOCKQ_CONTROL      0x1F
// PAGE 4      RF-Timing and Channel Redundancy
#define     REG_RX_WAIT             0x21
#define     REG_CHANNEL_REDUNDANCY  0x22
#define     REG_CRC_PRESET_LSB       0x23
#define     REG_CRC_PRESET_MSB       0x24
#define     RegTimeSlotPeriod     0x25
#define     RegMfOutSelect        0x26
#define     RFU27                 0x27
// PAGE 5      FIFO, Timer and IRQ-Pin Configuration
#define     RegFIFOLevel          0x29
#define     RegTimerClock         0x2A
#define     RegTimerControl       0x2B
#define     RegTimerReload        0x2C
#define     RegIRqPinConfig       0x2D
#define     RFU2E                 0x2E
#define     RFU2F                 0x2F
// PAGE 6      RFU
#define     REG_TYPESH                 0x31
#define     RFU32                 0x32
#define     RFU33                 0x33
#define     RFU34                 0x34
#define     RFU35                 0x35
#define     RFU36                 0x36
#define     RFU37                 0x37
// PAGE 7      Test Control
#define     RFU39                 0x39  
#define     RegTestAnaSelect      0x3A   
#define     RFU3B                 0x3B   
#define     RFU3C                 0x3C   
#define     RegTestDigiSelect     0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  						0x3F

#endif
