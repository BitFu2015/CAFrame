#include "CAFrame.h"

#define CH_CNT	2  //ת��ͨ��������
#define CV_TIM	8  //ת������

uint16_t  g_AdValue[CV_TIM][CH_CNT];   //�������ADCת�������Ҳ��DMA��Ŀ���ַ

void AdcInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_InitTypeDef  ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE );  //ʹ��ADC1ͨ��ʱ�ӣ������ܽ�ʱ��
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //72M/6=12M,ADC���ʱ�䲻�ܳ���14M
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);        //ʹ��DMA����
	
	//io����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_DeInit(ADC1);  //������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;        //ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode =ENABLE;        //ģ��ת��������ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;        //ģ��ת������������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;   //�ⲿ����ת���ر�
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;        //ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = CH_CNT;        //˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);        //����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���
	
	
	//����ָ��ADC�Ĺ�����ͨ�����������ǵ�ת��˳��Ͳ���ʱ��
	//ADC1,ADCͨ��x,�������˳��ֵΪy,����ʱ��Ϊ239.5����
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );                
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5 );

	
	// ����ADC��DMA֧��
	ADC_DMACmd(ADC1, ENABLE);        
	
	ADC_Cmd(ADC1, ENABLE);           //ʹ��ָ����ADC1

	ADC_ResetCalibration(ADC1);          //��λָ����ADC1��У׼�Ĵ���
	while(ADC_GetResetCalibrationStatus(ADC1));        //��ȡADC1��λУ׼�Ĵ�����״̬,����״̬��ȴ�
	
	ADC_StartCalibration(ADC1);                //��ʼָ��ADC1��У׼״̬
	while(ADC_GetCalibrationStatus(ADC1));                //��ȡָ��ADC1��У׼����,����״̬��ȴ�
	
	//ADC1  DMA1 Channel Config
	DMA_DeInit(DMA1_Channel1);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
	DMA_InitStructure.DMA_PeripheralBaseAddr =  (uint32_t)&ADC1->DR;  //DMA����ADC����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_AdValue;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //�ڴ���Ϊ���ݴ����Ŀ�ĵ�
	DMA_InitStructure.DMA_BufferSize = CH_CNT * CV_TIM;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�и����ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��
	
	DMA_Cmd(DMA1_Channel1, ENABLE);         //����DMAͨ��
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void AdcStart(void)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void AdcStop(void)
{
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
}


#define BEGIN_TEMPERATURE 40		//��ʼ�¶�ֵ(��)
#define REF_COUNT		120					//��ֵ����
#define ERROR_TEMPERATURE 80	//�����¶�

#define MAX_TEMP 1199 //80��
#define MIN_TEMP 0

//NTC 3450 10k ��ֵ��
const uint16_t g_ResTable[] =  \
{
23040, 21690, 20420, 19250, 18140, 17110, 16140, 15240, 14390, 13600, //-40 ~ -31
12860, 12160, 11500, 10890, 10310, 9762,  9252,  8770,  8317,  7891,  //-30 ~ -21
7489,  7111,  6754,  6417,  6100,  5800,  5517,  5250,  4997,  4758,	//-20 ~ -11
4532,  4319,  4116,  3925,  3744,  3572,  3409,  3255,  3109,  2970,	//-10 ~ -1
2838,  2713,  2594,  2481,  2374,  2272,  2175,  2083,  1995,  1912,	//0 ~ 9
1832,  1757,  1685,  1616,  1550,  1488,  1428,  1371,  1317,  1265,	//10 ~ 19
1216,  1169,  1124,  1081,  1039,  1000,  962,   926,   892,   859,		//20 ~ 29
827,797,768,740,714,688,664,640,618,596,//30~39
575,556,537,518,501,484,467,452,437,422,//40~49
409,395,382,370,358,347,336,325,315,305,//50~59
296,286,278,269,261,253,245,238,231,224,//60~69
217,211,205,199,193,187,182,177,172,167 //70~79
};

uint32_t GetTmpFromResAd(uint32_t res)
{
	uint32_t i,tmp;
	uint32_t a1,a2=0;
	
	res/=10;
	for(i=0;i<REF_COUNT;i++)
	{
		a1=a2;
		a2=g_ResTable[i];
		if(res > a2)
			break;
	}
	
	if(i==0) 
		return MIN_TEMP;
	else if(i == REF_COUNT)
		return MAX_TEMP;
	else;	

	//????	
	tmp= i;
	
	//????
	a1=a1-a2;
	a2=res - a2;
	a2*=100;
	a2=a2/a1;
	
	i=(a2+5)/10;
	
	a1=tmp;
	a1*=10;
	a1-=i;
	
	return a1;
}

//���ص�ѹ,�����¶�
uint32_t AdcGetVal(uint8_t ch)
{
	uint32_t val=0;
	uint32_t i;
	
	//ȡƽ��ֵ
	for(i=0;i<CV_TIM;i++)
		val+=g_AdValue[i][ch];
	val/=CV_TIM;
	
	i=val*5100;
	i/=(4096-val);
	
	return i;
}

uint16_t GetTemp(uint8_t ch)
{
	return GetTmpFromResAd(AdcGetVal(ch));
}


