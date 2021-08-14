/*********************************************
	文件名：Frame.c
	功  能：CAFrame应用程序框架 
	版  本：V2.2.4
	
	芯艺设计室(http://www.chipart.cn)  版权所有
	工程主页:http://www.chipart.cn/projects/prj_hit.asp?id=8
	
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
*********************************************/
#include "CAFrame.h"

volatile uint8_t g_TimerFlag;//1ms定时标记

//通用事件定义
static EVENT g_GenEvent[EVENT_MAX_COUNT];

//特殊事件定义
static EVENT_TIMER g_TimerEvent[TIMER_MAX_COUNT];//定时器事件
static EVENT_INPUT g_InputEvent[INPUT_MAX_COUNT];//输入事件

void InitApp(void);	//应用程序初始化

void frm_init(void)
{
	uint8_t i;
	
	for(i=0;i<TIMER_MAX_COUNT;i++)
		g_TimerEvent[i].id=0xff;
	
	for(i=0;i<INPUT_MAX_COUNT;i++)
		g_InputEvent[i].port=0xff;
		
	for(i=0;i<EVENT_MAX_COUNT;i++)
	{
		g_GenEvent[i].flag=0;
		g_GenEvent[i].Func=0;
	}
}

//向框架发送1ms定时信号
void frm_inc_timer_tick(void)
{
	g_TimerFlag++;	
}

//触发通用事件，在driver.c中调用
void frm_set_event(uint8_t event_id)
{
	if(event_id<EVENT_MAX_COUNT)
		g_GenEvent[event_id].flag=1;
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
	if(event_id<EVENT_MAX_COUNT)
		return g_GenEvent[event_id].flag;
	return 1;//错误
}

//输入事件触发函数
void frm_input_trigger(void)
{
	uint8_t i,tmp;
	
	for(i=0;i<INPUT_MAX_COUNT;i++)
	{
		if(g_InputEvent[i].port !=i)  //未安装事件或未初始化
			continue;
			
		tmp=drv_input(i);
		
		if(tmp==g_InputEvent[i].state)
		{
			if(g_InputEvent[i].filter_counter == 0) //保持状态
			{
				if(g_InputEvent[i].hold < 0xFFFF)
					g_InputEvent[i].hold++;					
				continue; 
			}
			
			g_InputEvent[i].filter_counter--;
			if(g_InputEvent[i].filter_counter==0)//进入保持状态
			{
				if((g_InputEvent[i].type == 2)||(g_InputEvent[i].type==tmp))
				{
					g_InputEvent[i].Func(g_InputEvent[i].hold);
				}
				g_InputEvent[i].hold=g_InputEvent[i].filter_config;
			}
		}
		else
		{
			g_InputEvent[i].state=tmp;
			g_InputEvent[i].filter_counter=g_InputEvent[i].filter_config;
		}
	}	
}

//获取滤波后输入状态
uint8_t frm_input(uint8_t port)
{
	return g_InputEvent[port].state;
}

//定时器事件触发函数
void frm_timer_trigger(void)
{
	uint8_t i,flag;
	PTIMER_TASK pTask;

	for(i=0;i<TIMER_MAX_COUNT;i++)
	{
		if(g_TimerEvent[i].id!=i)//未使能
			continue;

		if(++g_TimerEvent[i].cnt < g_TimerEvent[i].cfg)
			continue;
	  
	  //定时器绑定任务处理	
		pTask=g_TimerEvent[i].pTaskList;
		flag=0;
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
		
		if((flag==0) && (g_TimerEvent[i].Func != 0)) //如果没有任务被处理
	  	g_TimerEvent[i].Func();
		
		g_TimerEvent[i].cnt=0;
	}//for
}

void frm_event_trigger(void)
{
	uint8_t i;
	for(i=0;i<EVENT_MAX_COUNT;i++)
	{
		if(g_GenEvent[i].flag)//事件标记
		{
			if(g_GenEvent[i].Func!=0)
				g_GenEvent[i].Func();
			g_GenEvent[i].flag=0;
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

//输入端口初始化
void frm_install_input_event(uint8_t port,void (*fn)(uint16_t),uint8_t filter,uint8_t type)
{
	if(port >= INPUT_MAX_COUNT)
		return ;
		
	g_InputEvent[port].port=port;
	g_InputEvent[port].filter_config=filter;
	g_InputEvent[port].type=type;
	g_InputEvent[port].Func=fn;
	g_InputEvent[port].filter_counter=0;
	g_InputEvent[port].state=drv_input(port);
}

//定时器初始化
void frm_install_timer_event(uint8_t id,uint16_t tms,void (*fn)(void))
{
	if(id>=TIMER_MAX_COUNT)
		return ;
		
	g_TimerEvent[id].id=id;
	g_TimerEvent[id].cfg=tms;
	g_TimerEvent[id].Func=fn;
	g_TimerEvent[id].cnt=0;
	g_TimerEvent[id].pTaskList=0;
}

void frm_install_event(uint8_t id,void (*fn)(void))
{
	if(id>=EVENT_MAX_COUNT)
		return ;
	g_GenEvent[id].Func=fn;
}

//定时器使能
void frm_timer_enable(uint8_t id)
{
	if(id<TIMER_MAX_COUNT)
		g_TimerEvent[id].id=id;
}

//定时器禁用
void frm_timer_disable(uint8_t id)
{
	if(id < TIMER_MAX_COUNT)
		g_TimerEvent[id].id=0xff;
}

//定时器重新开始计数（复位）
void frm_timer_reset(uint8_t id)
{
	if(id < TIMER_MAX_COUNT)
		g_TimerEvent[id].cnt=0;
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
  
  pTask->busy=0; //默认关闭任务
  pTask->Func=fn;
  pTask->next=0;
  
  //如果列表为空
  if(g_TimerEvent[id].pTaskList==0)
  {
    g_TimerEvent[id].pTaskList=pTask;
    return ;
  }

  //找到最后一个TIMER_TASK
  p=g_TimerEvent[id].pTaskList;
	while(p->next)
	  p=p->next;
	
	p->next=pTask;  
}






