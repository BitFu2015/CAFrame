/*
 * Copyright (c) 2012-2026 Tong Changfu
 * Licensed under the Apache License, Version 2.0.
 */
 
/*
	文件名：Frame.c
	功  能：CAFrame应用程序框架 
	版  本：V2.3.0
	
	工程主页:http://www.chipart.cn/projects/prj_hit.asp?id=8
	开源地址1：
	开源地址2:
	
	历史记录
	芯艺 V1.0.0 2012-05-27  创建
	芯艺 V2.0.0 2013-10-08	规范化通用事件，定时器增加复位功能
	芯艺 V2.0.1 2013-10-09  增加定时任务功能
	芯艺 V2.0.2 2013-12-15	规范化通用事件
	芯艺 V2.1.0 2015-07-24  完善定时任务功能，可绑定到定时器使用(推荐使用方式)
	芯艺 V2.2.0 2015-11-05	为定时任务功能新增一套结构控制宏
	芯艺 V2.2.1 2017-06-25	增加主函数(MAIN)托管选项
	芯艺 V2.2.2 2017-10-07	输入事件中之前电平状态保持时间的问题修正
	芯艺 V2.2.3 2019-05-07  通用事件处理时无论是否安装处理函数，检测到标志后就清除
	芯艺 V2.2.4 2021-07-27  定时任务执行后后边的定时器暂不执行问题解决
	芯艺 V2.2.5 2022-02-25  使能定时器时复位定时器
	芯艺 V2.2.6 2025-04-12  增加阻塞运行时是否滴答计数选项
	芯艺 V2.3.0 2026-06-19  输入和定时器事件使用链表结构,强化定时任务功能
*/

#include "CAFrame.h"

volatile uint8_t g_TimerFlag;//1ms定时标记

//通用事件定义
static EVENT g_UserEvent[USER_EVENT_COUNT];

//特殊事件定义
static PEVENT_TIMER g_pTimerEvent;//定时器事件
static PEVENT_INPUT g_pInputEvent;//输入事件

void InitApp(void);	//应用程序初始化

void frm_init(void)
{
	uint8_t i;

	g_pTimerEvent=0;
	g_pInputEvent=0;
	
	for(i=0;i<USER_EVENT_COUNT;i++)
	{
		g_UserEvent[i].flag=0;
		g_UserEvent[i].Func=0;
	}
}

//向框架发送1ms定时信号
void frm_inc_timer_tick(void)
{
	#if FRM_SKIP_TICK_EN > 0
		g_TimerFlag=1;
	#else
		g_TimerFlag++;	
	#endif
}

//触发通用事件，在driver.c中调用
void frm_set_event(uint8_t event_id)
{
	if(event_id<USER_EVENT_COUNT)
		g_UserEvent[event_id].flag=1;
}

//清通用事件标记,在driver.c中调用
/*
void frm_clr_event(uint8_t event_id)
{
	if(event_id<EVENT_MAX_COUNT)
		g_GenEvent[event_id].flag=0;
}*/

uint8_t frm_get_event_state(uint8_t event_id)
{
	if(event_id<USER_EVENT_COUNT)
		return g_UserEvent[event_id].flag;
	return 1;//错误
}

//输入事件触发函数
static void frm_input_trigger(void)
{
	uint8_t tmp;
	PEVENT_INPUT pInputEvent=g_pInputEvent;
	
	//遍历所有输入事件
	while(pInputEvent)
	{
		tmp=drv_input(pInputEvent->port);
		
		if(tmp == pInputEvent->state) //保持上次输入状态
		{
			if(pInputEvent->filter_counter == 0) //保持状态
			{
				//记录保持时间
				if(pInputEvent->hold < 0xFFFF)
					pInputEvent->hold++;	
				pInputEvent=pInputEvent->next;				
				continue; 
			}
			
			pInputEvent->filter_counter--;
			if(pInputEvent->filter_counter==0)//进入保持状态
			{
				//所有边沿事件和当前边沿事件都触发
				if((pInputEvent->type == ET_ALL_EDGE)||(pInputEvent->type==tmp)) 
				{
					pInputEvent->Func(pInputEvent->hold);//调用事件函数并传递上一个状态保持时间
				}
				pInputEvent->hold=pInputEvent->filter_config;
			}
		}
		else//与上次输入状态不同时
		{
			pInputEvent->state=tmp;
			pInputEvent->filter_counter=pInputEvent->filter_config;
		}

		pInputEvent=pInputEvent->next;
	}	
}

//获取滤波后输入状态
uint8_t frm_input(uint8_t port)
{
	PEVENT_INPUT p;
	p=g_pInputEvent;
	while(p)
	{
		if(p->port == port)
			return p->state;
		p=p->next;
	}
	return 0;
}

//定时器事件触发函数
static void frm_timer_trigger(void)
{
	uint8_t flag;
	PTIMER_TASK pTask;
	PEVENT_TIMER pTimerEvent=g_pTimerEvent;

	//遍历所有定时器
	while(pTimerEvent)
	{
		if(pTimerEvent->id & 0x80)	//未使能
		{
			pTimerEvent=pTimerEvent->next;
			continue;	
		}

		if(++pTimerEvent->cnt < pTimerEvent->cfg)//定时未到时
		{
			pTimerEvent=pTimerEvent->next;
			continue;
		}
	  
		//定时器绑定任务处理	
		pTask=pTimerEvent->pTaskList;
		flag=0;//标记有任务被执行
		while(pTask)
		{
		  if(pTask->busy)//有任务在处理
		  {
		  	pTask->counter++; //计数器加一
		    if(pTask->Func) //如果有处理函数
		      pTask->Func(pTask);
		    flag=1;
		  }
		  pTask=(PTIMER_TASK)pTask->next;
		}
		
		if((flag==0) && (pTimerEvent->Func != 0)) //如果没有任务被处理
			pTimerEvent->Func();
		
		pTimerEvent->cnt=0;

		pTimerEvent=pTimerEvent->next;
	}//while
}

void frm_event_trigger(void)
{
	uint8_t i;
	for(i=0;i<USER_EVENT_COUNT;i++)//遍历所有用户事件
	{
		if(g_UserEvent[i].flag)//事件标记
		{
			if(g_UserEvent[i].Func!=0)
				g_UserEvent[i].Func();

			//drv_enter_critical();	//单条指令完成清零一个变量时无需加锁
			g_UserEvent[i].flag=0;
			//drv_exit_critical();
		}	
	}
}

void CAFrameInit(void)
{	
	drv_init();//驱动初始化
	frm_init();//框架初始化
	InitApp();//应用初始化
}

void CAFramePoll(void)
{		
	//1ms定时触发事件
	if(g_TimerFlag)
	{
		frm_timer_trigger();
		frm_input_trigger();
		
		drv_enter_critical();
		g_TimerFlag--;
		drv_exit_critical();
		
		drv_wdt_reset(); 
	}
	
	//通用事件触发
	frm_event_trigger();
}

#if MANAGE_MAIN > 0
int main(void)     
{
	CAFrameInit();
	while(1)
	{
		CAFramePoll();
	}//mail loop
}
#endif

//创建输入事件
void frm_create_input_event(uint8_t port,PEVENT_INPUT pEvent)
{
	PEVENT_INPUT p;

	pEvent->port=port;
	pEvent->Func=0;
	pEvent->next=0;
	
	//插入到链表末尾
	if(g_pInputEvent==0)
	{
		g_pInputEvent=pEvent;
	}
	else
	{
		p=g_pInputEvent;
		while(p->next)
			p=p->next;
		p->next=pEvent;
	}
}
//安装输入端口监测事件
void frm_install_input_event(uint8_t port,void (*fn)(uint16_t),uint8_t filter,uint8_t type)
{
	PEVENT_INPUT p=g_pInputEvent;
	while(p)
	{
		if(p->port == port)
			break;
		p=p->next;
	}

	if(p)
	{
		p->filter_config=filter;
		p->type=type;
		p->Func=fn;
		p->filter_counter=0;
		p->state=drv_input(port);		
	}
}

//创建定时器到框架
void frm_create_timer(uint8_t id,PEVENT_TIMER pTimer)
{
	PEVENT_TIMER p;

	pTimer->cfg=0;
	pTimer->id=id;
	//pTimer->cnt=0;
	pTimer->next=0;
	pTimer->pTaskList=0;

	if(g_pTimerEvent==0)
	{
		g_pTimerEvent=pTimer;
	}
	else
	{
		p=g_pTimerEvent;
		while(p->next)
			p=p->next;
		p->next=pTimer;
	}
}

//从编号获取定时器
static PEVENT_TIMER frm_get_timer_event(uint8_t id)
{
	PEVENT_TIMER p = g_pTimerEvent;
	while(p)
	{
		if((p->id & 0x7f) == id)
			return p;
		p = p->next;
	}
	return 0;
}
//定时器初始化
void frm_install_timer_event(uint8_t id,uint16_t tms,void (*fn)(void))
{
	PEVENT_TIMER p=frm_get_timer_event(id);
	if(p==0)
		return ;
	
	p->cfg=tms;
	p->Func=fn;
	p->cnt=0;
	p->pTaskList=0;
}

void frm_install_event(uint8_t id,void (*fn)(void))
{
	if(id>=USER_EVENT_COUNT)
		return ;
	g_UserEvent[id].Func=fn;
}

//定时器使能
void frm_timer_enable(uint8_t id)
{
	PEVENT_TIMER p=frm_get_timer_event(id);
	
	if(p)
	{
		p->id &= 0x7f;
		p->cnt=0;
	}
}

//定时器禁用
void frm_timer_disable(uint8_t id)
{
	PEVENT_TIMER p=frm_get_timer_event(id);
	if(p)
	{
		p->id |= 0x80;
		p->cnt=0;
	}
}

//定时器重新开始计数（复位）
void frm_timer_reset(uint8_t id)
{
	PEVENT_TIMER p=frm_get_timer_event(id);
	if(p)
	{
		p->cnt=0;
	}
}

//开始定时器任务
void frm_begin_timer_task(PTIMER_TASK pTask)
{
	if(pTask->busy==0)
	{
		pTask->busy=1;
		pTask->step=0;
		pTask->counter=0;
	}
}

//关闭定时器任务
void frm_end_timer_task(PTIMER_TASK pTask)
{
  pTask->busy=0;
}

//任务绑定到定时器,fn为定时任务处理函数
void frm_bind_timer_task(uint8_t id,PTIMER_TASK pTask,void (*fn)(PTIMER_TASK))
{
	PTIMER_TASK p;
	PEVENT_TIMER t = frm_get_timer_event(id);
	if(t==0) return ;

	pTask->busy=0; //默认关闭任务
	pTask->Func=fn;
	pTask->next=0;

	//如果列表为空
	if(t->pTaskList==0)
	{
	t->pTaskList=pTask;
	return ;
	}

	//找到最后一个TIMER_TASK
	p=t->pTaskList;
	while(p->next)
		p=p->next;
	
	p->next=pTask;  
}






