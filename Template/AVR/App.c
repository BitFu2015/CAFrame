//App.c - CAFrame AVR单片机工程模板示例程序

#include "CAFrame.h"

uint8_t g_RunMode=0;//0:关机　1:开机
uint8_t g_Buffer[SERIAL_BUFF_SIZE];    //通信用缓冲区
uint8_t g_Length; //缓冲区内数据长度

EVENT_TIMER g_TimerEvent1,g_TimerEvent2;
TIMER_TASK g_OpenTask,g_CloseTask;
EVENT_INPUT g_InputEvent1;

//串口接收事件
void SerialEvent(void)
{
    g_Length = drv_serial_input(g_Buffer);
}

//定时更新输入输出
void Timer1Event(void)
{
	LED_TOGGLE;
}

//定时器2，挂定时任务用
void Timer2Event(void)
{
	//当定时器绑定的定时任务执行时，此处不执行
	//....
}

//按键处理
void InputEvent1(uint16_t x)
{
	if(x>2000)//按钮长按(2秒以上)
	{
		if(g_RunMode)//处在开机状态
			frm_begin_timer_task(&g_CloseTask);//关机
	}
	else
	{
		if(g_RunMode==0)//处在关机状态
			frm_begin_timer_task(&g_OpenTask);//开始开机任务
	}
}

//等待并检测串口发回命令
int WaitAuthorize(int hold_time)
{
	if(g_Length > 0)
	{
		g_Length=0;
		if(g_Buffer[0]==0x55)
			return 1;
	}

	if(hold_time > 100) //超时失败
		return -1;

	return 0;//等待
}

//开机任务
void OpenTask(PTIMER_TASK pTask)
{
	int sta;
	
	TT_BEGIN(pTask);
	
	//等待远程授权
	g_Buffer[0]=0xAA;
	drv_serial_output(g_Buffer,1);//发送申请开机命令
	TT_TRY(pTask)
	{
		sta=WaitAuthorize(TT_HOLD_TIME(pTask));
		if(sta==0) TT_AGAIN;//等待
		if(sta<0) TT_EXIT(pTask);//失败，停止开机任务
	}

	drv_output(IO_Y1,1);//打开风机
	TT_DELAY(pTask,5); //延时5秒	
	drv_output(IO_Y2,1);//打开电热

	g_RunMode=1;
	TT_END(pTask);	
}

//关机任务
void CloseTask(PTIMER_TASK pTask)
{
	TT_BEGIN(pTask);
	
	drv_output(IO_Y2,0);//关闭电热
	
	TT_DELAY(pTask,30); //延时30秒
	
	drv_output(IO_Y1,0);//关闭风机
	
	g_RunMode=0;
	TT_END(pTask);		
}

/* 应用主程序初始化，该函数在框架程序(CAFrame)初始化时被调用
在这里要为框架程序创建和安装所有需要的事件及其处理函数    */
void InitApp(void)
{
	//创建配置定时器1
	frm_create_timer(0,&g_TimerEvent1);
	frm_install_timer_event(0,500,Timer1Event);
	
	//创建配置定时器2和定时任务
	frm_create_timer(1,&g_TimerEvent2);
	frm_install_timer_event(1,1000,Timer2Event);
	frm_bind_timer_task(1,&g_OpenTask,OpenTask);
	frm_bind_timer_task(1,&g_CloseTask,CloseTask);
	
	//创建配置输入事件
	frm_create_input_event(IO_X1,&g_InputEvent1);
	frm_install_input_event(IO_X1,InputEvent1,5,ET_FALLING_EDGE);//配置输入事件并安装回调函数

	//配置用户事件
	frm_install_event(EVENT_UART_ID,SerialEvent);//安装串口接收事件处理函数
}
