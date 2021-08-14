#include "CAFrame.h"

#define CH_CNT	2  //转换通道个数　
#define CV_TIM	8  //转换次数

uint16_t  g_AdValue[CV_TIM][CH_CNT];   //用来存放ADC转换结果，也是DMA的目标地址

void AdcInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_InitTypeDef  ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE );  //使能ADC1通道时钟，各个管脚时钟
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //72M/6=12M,ADC最大时间不能超过14M
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);        //使能DMA传输
	
	//io引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	ADC_DeInit(ADC1);  //将外设 ADC1 的全部寄存器重设为缺省值
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;        //ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode =ENABLE;        //模数转换工作在扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;        //模数转换工作在连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;   //外部触发转换关闭
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;        //ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = CH_CNT;        //顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);        //根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器
	
	
	//设置指定ADC的规则组通道，设置它们的转化顺序和采样时间
	//ADC1,ADC通道x,规则采样顺序值为y,采样时间为239.5周期
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );                
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5 );

	
	// 开启ADC的DMA支持
	ADC_DMACmd(ADC1, ENABLE);        
	
	ADC_Cmd(ADC1, ENABLE);           //使能指定的ADC1

	ADC_ResetCalibration(ADC1);          //复位指定的ADC1的校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1));        //获取ADC1复位校准寄存器的状态,设置状态则等待
	
	ADC_StartCalibration(ADC1);                //开始指定ADC1的校准状态
	while(ADC_GetCalibrationStatus(ADC1));                //获取指定ADC1的校准程序,设置状态则等待
	
	//ADC1  DMA1 Channel Config
	DMA_DeInit(DMA1_Channel1);   //将DMA的通道1寄存器重设为缺省值
	DMA_InitStructure.DMA_PeripheralBaseAddr =  (uint32_t)&ADC1->DR;  //DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_AdValue;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //内存作为数据传输的目的地
	DMA_InitStructure.DMA_BufferSize = CH_CNT * CV_TIM;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在循环缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有高优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道
	
	DMA_Cmd(DMA1_Channel1, ENABLE);         //启动DMA通道
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


#define BEGIN_TEMPERATURE 40		//开始温度值(负)
#define REF_COUNT		120					//阻值数量
#define ERROR_TEMPERATURE 80	//错误温度

#define MAX_TEMP 1199 //80度
#define MIN_TEMP 0

//NTC 3450 10k 阻值表
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

//返回电压,返回温度
uint32_t AdcGetVal(uint8_t ch)
{
	uint32_t val=0;
	uint32_t i;
	
	//取平均值
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


