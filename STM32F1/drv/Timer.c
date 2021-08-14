//timer.c
#include"caframe.h"

//1ms定时器
volatile uint32_t g_SysTimer;

//初始化
void InitTick(void)
{
	g_SysTimer=0;	 
	SysTick_Config(72000);	
}

//系统定时器中断函数(1ms中断)
void SysTick_Handler( void )
{
	g_SysTimer++;
	frm_inc_timer_tick();		
}

//获取当前定时值
uint32_t GetCurrentTick(void)
{
	uint32_t ret;
	CLI();
	ret=g_SysTimer;
	SEI();
	return ret;
}
//计算过去时间
uint32_t GetPassedTick(uint32_t t)
{
	uint32_t cur=GetCurrentTick();

	if(cur>=t)
		return (cur-t);

	return (cur+(0xffffffff-t));
}
//延时
void SysDelayMs(uint32_t t)
{
	uint32_t tick=GetCurrentTick();
	while(GetPassedTick(tick)<t);
}
//////////////////////////////////////////

//粗略延时毫秒函数(系统时钟：72MHz) 
void DelayMs(uint32_t t)
{
	static uint32_t MsDelayCnt;
	while(t--)
	for(MsDelayCnt=0;MsDelayCnt<10280;MsDelayCnt++);
}

//粗略延时微秒函数(系统时钟:72MHz)
void DelayUs(uint32_t t)
{
	static uint32_t UsDelayCnt;
	while(t--)
	for(UsDelayCnt=0;UsDelayCnt<10;UsDelayCnt++);
}

