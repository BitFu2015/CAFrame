//timer.c
#include"caframe.h"

//1ms��ʱ��
volatile uint32_t g_SysTimer;

//��ʼ��
void InitTick(void)
{
	g_SysTimer=0;	 
	SysTick_Config(72000);	
}

//ϵͳ��ʱ���жϺ���(1ms�ж�)
void SysTick_Handler( void )
{
	g_SysTimer++;
	frm_inc_timer_tick();		
}

//��ȡ��ǰ��ʱֵ
uint32_t GetCurrentTick(void)
{
	uint32_t ret;
	CLI();
	ret=g_SysTimer;
	SEI();
	return ret;
}
//�����ȥʱ��
uint32_t GetPassedTick(uint32_t t)
{
	uint32_t cur=GetCurrentTick();

	if(cur>=t)
		return (cur-t);

	return (cur+(0xffffffff-t));
}
//��ʱ
void SysDelayMs(uint32_t t)
{
	uint32_t tick=GetCurrentTick();
	while(GetPassedTick(tick)<t);
}
//////////////////////////////////////////

//������ʱ���뺯��(ϵͳʱ�ӣ�72MHz) 
void DelayMs(uint32_t t)
{
	static uint32_t MsDelayCnt;
	while(t--)
	for(MsDelayCnt=0;MsDelayCnt<10280;MsDelayCnt++);
}

//������ʱ΢�뺯��(ϵͳʱ��:72MHz)
void DelayUs(uint32_t t)
{
	static uint32_t UsDelayCnt;
	while(t--)
	for(UsDelayCnt=0;UsDelayCnt<10;UsDelayCnt++);
}

