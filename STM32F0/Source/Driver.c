/*********************************************
	�ļ�����Driver.c
	��  �ܣ�STM32F0 CAFrame������ģ�����
	
	о�������(http://www.chipart.cn)  ��Ȩ����
	������ҳ:http://www.chipart.cn/projects/prj_hit.asp?id=8
	
	��ʷ��¼
	о��  2015-08-11  ����
*********************************************/
#include "CAFrame.h"

uint8_t g_IntFlag=0;   //���жϹرռ�����

/****************************************************************************
//ͨ�ýӿں���
****************************************************************************/
//ϵͳ��ʱ���ж� 1ms�ж�
void SysTick_Handler(void)
{
  frm_inc_timer_tick();
}

//�豸��ʼ��
void drv_init(void)
{
	GPIO_InitTypeDef        GPIO_InitStructure;
	
	//ʹ�ö˿�ʱ������
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOA, ENABLE);

  //LED O1,O2���ų�ʼ��
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15|GPIO_Pin_13|GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	//O3,O4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//�������ų�ʼ�� IN1,IN2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	//IN3,IN4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_6;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	#ifdef DEBUG_MODE
	DebugPrintInit();
	#endif
	
	//ϵͳʱ��1ms
	SysTick_Config(SystemCoreClock / 1000);
}

void drv_enter_critical(void)
{
	__disable_irq();
	g_IntFlag++;
}

void drv_exit_critical(void)
{
  if(--g_IntFlag == 0)
    __enable_irq() ;
}

void drv_wdt_reset(void)
{
 
}
/////////////////////////////////////////////////////////////////////////////


/****************************************************************************
//��������/���
****************************************************************************/
//������ƽӿں���
void drv_output(uint8_t port,uint8_t val)
{
	if(val)
	{
		if(port==IO_Y1)
			GPIOC->BSRR=GPIO_Pin_14;
		else if(port==IO_Y2)
			GPIOC->BSRR=GPIO_Pin_13;
		else if(port==IO_Y3)
			GPIOB->BSRR=GPIO_Pin_9;
		else if(port==IO_Y4)
			GPIOB->BSRR=GPIO_Pin_8;
		else;
	}
	else
	{
		if(port==IO_Y1)
			GPIOC->BRR=GPIO_Pin_14;
		else if(port==IO_Y2)
			GPIOC->BRR=GPIO_Pin_13;
		else if(port==IO_Y3)
			GPIOB->BRR=GPIO_Pin_9;
		else if(port==IO_Y4)
			GPIOB->BRR=GPIO_Pin_8;
		else;
	}
}

//��ȡ��������
uint8_t drv_input(uint8_t port)
{
	uint8_t ret=0;
	
	if(port==IO_X1)
	{
		if(GPIOA->IDR & GPIO_Pin_11)
			ret=1;
	}
	else if (port==IO_X2)
	{
		if(GPIOA->IDR & GPIO_Pin_12)
			ret=1;
	}
	else if(port==IO_X3)
	{
		if(GPIOF->IDR & GPIO_Pin_6)
			ret=1;		
	}
	else if(port==IO_X4)
	{
		if(GPIOF->IDR & GPIO_Pin_7)
			ret=1;
	}		
		
	return ret;
}

/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//EEPROM��������
****************************************************************************/
void drv_eeprom_read(uint16_t addr,uint8_t *buf,uint8_t size)
{

}

void drv_eeprom_write(uint16_t addr,uint8_t *buf,uint8_t size)
{

}
/////////////////////////////////////////////////////////////////////////////

/****************************************************************************
//���ڸ�ʽ����ӡ����
****************************************************************************/
#ifdef DEBUG_MODE

//��ʼ�����Դ�ӡ�������
void DebugPrintInit(void)
{	
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	//ʱ������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	//USART1 Pins configuration 
  //Connect pin to Periph
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1); 
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);  
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 |GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 
	
	//RS485 DIR����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	RS485_TX_SEL;
	
	USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; 
	//��ʼ��USART1
  USART_Init(USART1, &USART_InitStructure);	
	USART_Cmd(USART1, ENABLE);//USART1ʹ��
}

//��׼��ӡ�������
int fputc(int ch, FILE *f)
{
  USART_SendData(USART1, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

  return ch;
}

struct __FILE 
{
	int handle; 
	//... 
};

FILE __stdout;

#endif
/////////////////////////////////////////////////////////////////////////////
