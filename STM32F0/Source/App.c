/**************************************************
	文件名：App.c
	功  能：CAFrame 定时任务应用示例 
	MCU:STM32F030C8T6
	时钟:48MHz(外接8MHz晶振)
	版  本：V1.0
	
	芯艺设计室(http://www.chipart.cn)  版权所有
	工程主页:http://www.chipart.cn/projects/prj_hit.asp?id=8

	
	历史记录 
	芯艺 V1.0 2015-08-12  创建
**************************************************/ 
#include "CAFrame.h"

/*　说明：
定时任务用于实现延时分步操作功能。
例如我们需要实现这样一个功能：在一个温控系统中，电加热棒打开前先打开风机再延时10秒后打开电热，
关闭时先关闭电加热棒，再延时30秒关风机，并且要求关闭后在3分钟内不得再次打开。
本示例使用CAFrame定时任务实现了此功能；
*/

//定时任务对象定义
TIMER_TASK g_OpenTask,g_CloseTask;

uint8_t g_RunStatus=0;			//当前运行状态
uint8_t g_ControlStatus=0;	//当前控制输入状态

void Timer1Event(void)
{
	LED_TOGGLE; 

	printf("RunStatus:%d,ControlStatus:%d\r\n",g_RunStatus,g_ControlStatus);
}

//开机任务处理函数
void OpenTask(PTIMER_TASK pTask)
{
	if(pTask->step == 0)
	{
		drv_output(IO_Y1,1);//打开风机
		frm_timer_task_nextstep(pTask); //切换到下一步
	}
	else if(pTask->step == 1)
	{
		if(pTask->counter >= 10) //延时10秒 , pTask->counter 表示当前状态已保持的定时时长
		{
			drv_output(IO_Y2,1);//打开电热
			frm_end_timer_task(pTask); //停止该定时任务
			g_RunStatus=1;//开机完成
		}
	}
	else;
}

//关机任务处理函数
void CloseTask(PTIMER_TASK pTask)
{
	if(pTask->step == 0)
	{
		drv_output(IO_Y2,0);//关闭电热
		frm_timer_task_nextstep(pTask);
	}
	else if(pTask->step == 1)
	{
		if(pTask->counter > 30) //等待30秒关闭风机
		{
			drv_output(IO_Y1,0);//关闭风机
			frm_timer_task_nextstep(pTask);
		}
	}
	else if(pTask->step == 2)
	{
		if(pTask->counter > 180) //等待3分钟避免频繁启动
		{
			frm_end_timer_task(pTask);
			g_RunStatus=0;//关机完成
		}
	}
	else ;
}

//定时器1事件处理函数
void TaskTimerEvent(void)
{
	//注：如果定时器所邦定的定时任务处于执行状态，则以下代码不会得到执行
	//只有所有邦定到此定时器的定时任务均处于停止时才会执行此函数
	
	//如果控制状态改变
	if(g_ControlStatus != g_RunStatus)
	{
		if(g_RunStatus)//正处在开机状态
			frm_begin_timer_task(&g_CloseTask);
		else
			frm_begin_timer_task(&g_OpenTask);
	}
}

void InputEvent1(uint16_t x)
{
	//控制开关状态切换
	g_ControlStatus=g_ControlStatus?0:1; 
}

/*应用主程序初始化函数
在框架程序(frame.c)中调用的唯一应用接口函数
在这里要为框架程序安装所有需要的事件处理函数*/
void InitApp(void)
{
	//普通定时事件
	frm_install_timer_event(0,2000,Timer1Event);	 	//控制操作时钟
	
	//控制定时器
	frm_install_timer_event(1,1000,TaskTimerEvent);
  frm_bind_timer_task(1,&g_OpenTask,OpenTask); //邦定开机任务
  frm_bind_timer_task(1,&g_CloseTask,CloseTask);//邦定关机任务
	
	//安装控制按键输入事件
	//IO_X1输入下降沿触发事件,低电平至少保持5ms
	frm_install_input_event(IO_X1,InputEvent1,5,ET_FALLING_EDGE);
}


