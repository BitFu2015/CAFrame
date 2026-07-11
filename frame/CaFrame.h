/*
 * Copyright (c) 2012-2026 Tong Changfu
 * Licensed under the Apache License, Version 2.0.
 */
 
 //CAFrame.h - 框架主声明文件
 
#ifndef CAFRAME_H
#define CAFRAME_H

#include "Driver.h"
#include "cfg.h"

#define USE_CAFRAME		//CAFrame应用标志

/** \mainpage CAFrame单片机C语言程序开发框架
 
 \author 芯艺工作室 - http://www.chipart.cn
 
 \par 简介 
	CAFrame 单片机C语言程序开发框架可用于不同的单片机，如AVR,STM32,STM8等单片机控制器的软件开发。框架实现了硬件驱动和应用逻辑的分开，
	以事件驱动的方式编写应用程序，使得应用功能的编写就如同Visual Basic程序那样简单且易维护扩展。
	框架程序共由三个头文件（CAFrame.h Cfg.h Driver.h）和三个源文件(Driver.c Frame.c App.c)组成
	其层次结构如图： \n
 	<img src=layer.jpg> \n
 	Driver.c中的代码用于处理与硬件相关的底层驱动，Driver.h文件中声明与硬件相关的一些符号定义和用户自定义驱动函数 \n
 	驱动程序的编写主要通过修改这两个文件来实现. \n \n
	CAFrame.h是框架标准符号、结构、函数的声明处，Frame.c是框架的实现文件，无论任何时候都不应该修改这两个文件。 \n	
 	注意： CAFrame.h文件中声明的以[drv_]为前缀的函数是框架所必须或者标准化用的驱动接口函数，在移置时这些函数应在driver.c中实现. \n \n
	App.c是应用逻辑实现文件，其中一个固定的函数InitApp在框架初始化后第一个被调用，在这个函数中设置好所有需要的事件处理函数。作为一个完整的app.c文件，下面的示例说明了开发者需要做些什么
 	\code
 	#include "CAFrame.h"
	EVENT_TIMER g_Timer0;
 	void TimerEvent(void)//定时器事件处理函数
 	{
 		LED_FLASH;
 	}
 	void InitApp(void)
 	{
		frm_create_timer_event(0,&g_Timer0);//创建定时器
 		frm_install_timer_event(0,200,TimerEvent);//为定时器安装处理函数,定时触发周期为200ms
 	}
 	\endcode
	
	\par 程序运行优先级
	CAFrame并非是一个操作系统,而是一个典型的前后台程序结构，程序运行优先级只有中断级（INT_LEVEL）和应用级（APP_LEVEL).
	driver.c中一部分硬件驱动程序在中断中调用,它们工作在INT_LEVEL级,但这些对于APP来说是透明的，无需考虑的，
	所有的接口函数以及用户代码（app.c)均运行在同一个优先级（APP_LEVEL），为此用户程序不会相互中断。
	
	\par CAFrame配置	
	CAFrame的配置文件为cfg.h，配置中“用户事件ID定义列表”中列出所用到的所有用户事件ID，值从0开始递增,数量必须等于USER_EVENT_COUNT.
	\code
	#define UINT uint32_t			//定义系统无符号整数类型

	#define	MANAGE_MAIN	1			//主程序托管
	#define FRM_SKIP_TICK_EN  0		//阻塞时跳过系统计数

	#define USER_EVENT_COUNT 2		//用户事件个数

	//用户事件ID定义列表,值从0开始递增(0,1,2...)
	#define EVENT_ADC_ID 0		
	#define EVENT_UART_ID 1
	\endcode
	MANAGE_MAIN 的值决定整个项目入口函数(main)是否托管给框架，如果值是1，就等于用户无需实现main函数，将main函数托管给了框架。
	如果MANAGE_MAIN定义为0,称为非托管应用，需要在用户程序中实现一个如下的main函数:
	\code
	int main(void)
	{
		CAFrameInit();//框架初始化
		
		while(1)
		{
			//..   //主循环中的其它任务
			
			CAFramePoll();	//框架循环
		}
		return 0;
	}	
	\endcode
	
	\par 用户事件应用
	事件是框架中驱动层与应用层同步的主要方式，框架支持三种事件，分别是定时器事件、输入事件和用户事件，定时器事件和输入事件是框架固定的
	特殊事件，而用户事件是用户编写外设驱动与应用层同步的主要方式，如UART,ADC的中断中可以通过用户事件的方式在应用层触发事件。用户事件运行原理如图： \n 
	<img src=event.jpg> \n
	首先需要在配置文件(cfg.h)中定义事件ID,如
	\code 
	#define EVENT_UART_ID 1
	\endcode
	然后在驱动中断中触发事件：
	\code
	ISR
	{
		...
		frm_set_event(EVENT_UART_ID);
		...
	}
	\endcode
	如果在InitApp函数中已经用frm_install_event安装了事件处理函数，那么中断中一触发这个事件，框架会在中断结束后在后台（APP_LEVEL级）调用
	事件处理函数。
\n\n
	\par 软件许可
	* <pre>
	Copyright 2012-2026 Tong Changfu 

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0 

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
	* </pre>
  */
  

/** \defgroup InputDriver 开关输入接口

	开关输入处理方式可以有两种方式
	\par 事件驱动方式
	
	示例如下(app.c)：
	\code
	EVENT_INPUT g_InputX1;
	void InputEvent1(uint16_t x)
	{
		//x为事件产生前的电平状态保持时间
		LED_TOGGLE;
	}	 
	void InitApp(void)
	{
		frm_create_input(IO_X1,&g_InputX1);//创建输入事件
		frm_install_input_event(IO_X1,InputEvent1,5,ET_ALL_EDGE);//配置输入事件并安装回调函数
		// ... 其它事件函数的安装
	}
	\endcode
	
	\par 定时器内扫描方式
	
	在定时器内通过drv_input或frm_input函数检测输入状态，drv_input直接读取端口电平状态，而frm_input为框架滤波后的输入状态;
	\note 如果使用frm_input函数，则必须在InitApp中创建事件并将相应端口输入事件配置成ET_DISABLE模式,如：
	
	\code					
	EVENT_INPUT g_InputX1;
	
	void InitApp(void)
	{
		//....
		frm_create_input(IO_X1,&g_InputX1);//创建输入事件
		frm_install_input_event(IO_X1,0,5,ET_DISABLE); 
		//....
	}
	\endcode
*/

/** \defgroup OutputDriver 输出控制接口
	输出控制接口函数drv_output以框架统一的方式控制单片机IO口的输出行为，\n 这些IO口被编号成IO_Y1,IO_Y2 ...  \n
	drv_output函数实现编号对应的IO的输出控制行为。
*/
/** \defgroup TimerDriver  定时器接口

	定时器操作十分简单，一个最简单的示例如下(app.c)：
	\code
	EVENT_TIMER g_Timer1;
	void TimerEvent1(void)
	{
		LED_TOGGLE;//发光管控制端口取反，使其闪烁
	}	
	void InitApp(void)
	{
		frm_create_timer(0,&g_Timer1);//框架中创建定时器
		frm_install_timer_event(0,300,TimerEvent1);//配置定时器并安装事件处理函数,定时触发周期为300ms
		// ... 其它初始化操作
	}
	\endcode
*/

/** \defgroup GenDriver 基础驱动函数

	实现框架的基础驱动接口函数，框架将调用这些接口，框架移植到新的硬件上时必须在driver.c中实现这些函数和对应功能。
*/

/** \defgroup StdDriver 标准驱动函数

	框架定义的标准化驱动函数，为了使驱动层和应用层接口程序标准化，框架定义了一些标准化的函数，
	框架移植到新的硬件上时应在driver.c中实现这些函数，框架本身不调用这些函数，为此在项目中用不到
	时可忽略函数内容，简单返回即可。
*/

/** \defgroup GenFrame 框架事件触发接口

	在驱动中触发框架用的接口函数
*/

/** \defgroup TimerTask 定时任务管理
	定时任务用于实现时序逻辑功能
	例如我们需要实现这样一个功能：在一个温控系统中，电加热棒打开前先打开风机再延时10秒后打开电热，
	关闭时先关闭电加热棒，再延时30秒关风机，并且要求关闭后在3分钟内不得再次打开。
	使用定时任务实现此功能的代码如下：
	\code
	#include "CAFrame.h"

	EVENT_TIMER g_Timer1,g_Timer2;
	EVENT_INPUT g_InputX1;

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
		TT_BEGIN(pTask); 

		drv_output(IO_Y1,1);//打开风机

		TT_DELAY(pTask,10);//延时10秒
		
		drv_output(IO_Y2,1);//打开电热
		g_RunStatus=1;//开机完成状态

		TT_END(pTask);
	}
	
	//关机任务处理函数
	void CloseTask(PTIMER_TASK pTask)
	{
		TT_BEGIN(pTask);

		drv_output(IO_Y2,0);//关闭电热
		
		TT_DELAY(pTask,30); //延时30秒
		
		drv_output(IO_Y1,0);//关闭风机
		
		TT_DELAY(pTask,180);//等待3分钟避免频繁启动

		g_RunStatus=0;//关机完成状态
		
		TT_END(pTask);
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
	
	void InitApp(void)
	{
		//框架内创建一个定时器
		frm_create_timer(0,&g_Timer1);
		//普通定时事件，此定时器与此功能无关，仅用于调试
		frm_install_timer_event(0,2000,Timer1Event);
		
		//控制定时器
		frm_create_timer(1,&g_Timer2);
		frm_install_timer_event(1,1000,TaskTimerEvent);
		frm_bind_timer_task(1,&g_OpenTask,OpenTask); //邦定开机任务
		frm_bind_timer_task(1,&g_CloseTask,CloseTask);//邦定关机任务
		
		//创建并安装控制按键输入事件
		//IO_X1输入下降沿触发事件,低电平至少保持5ms
		frm_create_input_event(IO_X1,&g_InputX1);
		frm_install_input_event(IO_X1,InputEvent1,5,ET_FALLING_EDGE); 
	}

 	\endcode	
*/


/////////////////////////////////////////////////////////
//框架(frame.c)接口函数//////////////////////////////////
/*
	输入事件管理数据结构(仅框架程序使用)
*/
typedef struct tag_input_event
{
	//端口号
	uint8_t port; 	
	//触发类型 0:下降沿触发，1：上升沿触发  2：边沿触发  3：不触发(用于循检)	
	uint8_t type;		
	//当前状态(0或1)
	uint8_t state;	
	//保持时间(ms)
	uint16_t hold;	
	//滤波设置(ms)
	uint8_t filter_config;	
	//滤波计数器
	uint8_t filter_counter;	
	//事件处理函数	
	void (*Func)(uint16_t);	

	struct tag_input_event * next;
}EVENT_INPUT,*PEVENT_INPUT;
//输入事件触发类型符号定义
#define ET_FALLING_EDGE	0
#define ET_RISING_EDGE	1
#define ET_ALL_EDGE			2
#define ET_DISABLE			3

/*
	定时任务管理数据结构
*/
typedef struct tag_timer_task
{
	uint8_t busy;		//互斥检测标记	
	int step; 	//当前执行步骤
	int counter; //延时用计数器
	
	void (*Func)(struct tag_timer_task *);//任务处理函数
	
	struct tag_timer_task *next; //下一个结构
}TIMER_TASK,*PTIMER_TASK;

/*
	定时器管理数据结构(仅在框架程序使用)
*/
typedef struct tag_timer_event
{
	//定时器编号
	uint8_t id;			
	//定时设定值
	uint16_t cfg;		
	//定时计数器
	uint16_t cnt;		
	//事件处理函数
	void (*Func)(void);
	
	//定时任务列表
	struct tag_timer_task *pTaskList;	
	
	struct tag_timer_event *next;
}EVENT_TIMER,*PEVENT_TIMER;

/*
	用户事件管理数据结构
*/
typedef struct tag_event
{
	//事件ID
	uint8_t id;

	//事件发生标记
	uint8_t flag;
	//事件处理函数
	void (*Func)(void);

//	struct tag_event *next;
}EVENT,*PEVENT;

/** \ingroup InputDriver
	\brief 读取开关输入端口状态，用于读取滤波后的开关输入状态
	\param[in] port 端口:IO_X1 ~ IO_X6
	\return 有信号返回1  无信号返回0
*/
uint8_t frm_input(uint8_t port);

/** \ingroup InputDriver
	\brief 创建开关输入事件
	\param[in] port 监测端口:IO_X1 ~ IO_X...
	\param[in] pEvent 输入事件结构指针
*/
void frm_create_input_event(uint8_t port,PEVENT_INPUT pEvent);

/** \ingroup InputDriver
	\brief 安装开关输入事件处理函数
	\param port 端口号:IO_X1 ~ IO_X...
	\param fn 事件处理函数，原型: void func(uint16_t t)<br>参数t为之前电平状态保持的时间(ms)，如果保持时间大于0xFFFF则t返回0xFFFF;
	\param filter 滤波计数器，以毫秒为单位的信号确认时间
	\param type ET_FALLING_EDGE：下降沿触发<BR>  ET_RISING_EDGE：上升沿触发 <BR>ET_ALL_EDGE：边沿触发<BR>ET_DISABLE：事件触发禁止(检测应用时使用)
*/
void frm_install_input_event(uint8_t port,void (*fn)(uint16_t),uint8_t filter,uint8_t type);

/** \ingroup TimerDriver
	\brief 创建定时器
	\param id 用于指定器编号，范围 0,1,2 ... 127，须保证唯一
	\param pTimer 定时器结构指针
*/
void frm_create_timer(uint8_t id,PEVENT_TIMER pTimer);

/** \ingroup TimerDriver
	\brief 安装定时器事件处理函数
	\param id 定时器编号
	\param tms 定时器定时周期(毫秒为单位)
	\param fn 定时事件处理函数<br>原型:void func(void)
*/
void frm_install_timer_event(uint8_t id,uint16_t tms,void (*fn)(void));

/** \ingroup TimerDriver
	\brief 开启指定定时器
	\param id 定时器编号
*/
void frm_timer_enable(uint8_t id);

/** \ingroup TimerDriver
	\brief 禁止指定定时器
	\param id 定时器编号
*/
void frm_timer_disable(uint8_t id);

/** \ingroup TimerDriver
	\brief 定时器重新开始计数
	\param id 定时器编号
*/
void frm_timer_reset(uint8_t id);

/** \ingroup GenFrame
	\brief 安装用户事件
	\param id 事件ID
	\param fn 事件处理函数<br>原型：void func(void)
*/
void frm_install_event(uint8_t id,void (*fn)(void));

/** \ingroup GenFrame
	\brief 框架1ms滴答驱动，此函数必须在驱动中每一毫秒调用一次
	\note 用于支持框架的输入检测和定时触发功能
*/
void frm_inc_timer_tick(void);

/** \ingroup GenFrame 
	\brief 触发用户事件，此函数通常在驱动的中断中调用，用于实现对应事件的触发
	\param id 触发事件ID
*/
void frm_set_event(uint8_t id);

/** \ingroup GenFrame
	\brief 查询指定事件的触发状态
	\param id 查询事件ID
*/
uint8_t frm_get_event_state(uint8_t id);


/////////////////////////////////////////////////////////
//驱动(driver.c)必须实现的接口函数///////////////////////
/** \ingroup GenDriver
	\brief 框架初始化前调用，用于初如化硬件设备。在驱动(driver.c)中必须实现此函数。
*/
void drv_init(void);

/** \ingroup GenDriver
	\brief 看门狗复位，在框架主循环中调用，在驱动(driver.c)中必须实现此函数。
*/
void drv_wdt_reset(void);

/** \ingroup GenDriver
	\brief 安全关闭中断，在执行一段不希望被中断的代码时使用，此函数需与drv_exit_critical配对使用。在驱动(driver.c)中必须实现此函数。
*/
void drv_enter_critical(void);

/** \ingroup GenDriver
	\brief 退出中断保护代码段,此函数需与drv_enter_critical配对使用。在驱动(driver.c)中必须实现此函数。
*/
void drv_exit_critical(void);

/** \ingroup InputDriver
	\brief 读取开关输入端口状态，直接读取端口当前的输入信号，在驱动(driver.c)中必须实现此函数。
	\param port 端口:IO_X1 ~ IO_X6
	\return 有信号返回1  无信号返回0
*/
uint8_t drv_input(uint8_t port);

/** \ingroup OutputDriver
	\brief 开关量输出控制函数，在驱动(driver.c)中必须实现此函数。
	\param port 输出端口号(IO_Y1 ~ IO_Y6)
	\param val  输出值  1：输出继电器吸合  0：输出继电器断开
*/
void drv_output(uint8_t port,uint8_t val);

/** \ingroup StdDriver
	\brief 安全读取EEPROM接口函数,在驱动(driver.c)中必须实现此函数。
	\param addr 读取地址
	\param buf 读取缓冲区
	\param size 读取长度(字节)
*/
void drv_eeprom_read(uint16_t addr,uint8_t *buf,uint8_t size);

/** \ingroup StdDriver
	\brief 安全写入EEPROM接口函数,在驱动(driver.c)中必须实现此函数。
	\param addr 写入地址
	\param buf 写入缓冲区
	\param size 写入长度(字节)
*/
void drv_eeprom_write(uint16_t addr,uint8_t *buf,uint8_t size);

/** \ingroup StdDriver
	\brief 获取当前系统滴答计数值
	\return 返回当前系统滴答计数值
*/
UINT drv_get_current_tick(void);

/** \ingroup StdDriver
	\brief 获取过去某一时刻到现在的系统滴答个数
	\param t 过去某一时刻记录的系统滴答计数值
	\return 返回该时间段内发生的系统滴答数
*/
UINT drv_get_past_tick(UINT t);

/** \ingroup TimerTask
	\brief 开始执行定时任务
	\param pTask 定时任务对象指针
*/
void frm_begin_timer_task(PTIMER_TASK pTask);

/** \ingroup TimerTask
	\brief 停止指定的定时任务
	\param pTask 定时任务对象指针
*/
void frm_end_timer_task(PTIMER_TASK pTask);

/** \ingroup TimerTask	
 * 设置定时任务的当前执行步骤
 * 
 * \note 此函数为兼容老版程序而保留，新设计中应避免使用！　
 * \hideinitializer
*/
#define frm_timer_task_setstep(T,S) (T->step=S,T->counter=0)

/** \ingroup TimerTask	
 * 设置定时任务转换到下一个执行步骤　
 * 
 * \note 此函数为兼容老版程序而保留，新设计中应避免使用！
 * \hideinitializer
*/
#define frm_timer_task_nextstep(T)  (T->step++,T->counter=0)

/** \ingroup TimerTask
	\brief 定时任务绑定到指定定时器
	\param id 定时器序号
	\param pTask 定时任务对象地址
	\param fn 定时任务处理函数
*/
void frm_bind_timer_task(uint8_t id,PTIMER_TASK pTask,void (*fn)(PTIMER_TASK));

/** \ingroup TimerTask	
 * 定时任务开始处理，在定时任务处理函数的开始处调用；
 * \note 此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象地址
 * \hideinitializer
*/
#define TT_BEGIN(T) switch(T->step){case 0:

/** \ingroup TimerTask
 * 定时任务结束处理，在定时任务的结束处调用；用此宏结束的定时任务顺序执行完成后退出；
 * \note 该宏与TT_LOOP互斥，任务结束处只能且必须调用TT_END和TT_LOOP其中之一
 * \note 此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象地址
 * \hideinitializer
*/	
#define TT_END(T) default:frm_end_timer_task(T);break;}

/** \ingroup TimerTask	
 * 定时任务循环处理,在定时任务的结束处调用；用此宏结束的定时任务顺序执行完成后自动重新回到开始处继续执行，
 * 不会自动结束；
 * \note 该宏与TT_END互斥，任务结束处只能且必须调用TT_END和TT_LOOP其中之一
 * \note 此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象指针
 * \hideinitializer
*/
#define TT_LOOP(T) T->counter=0;default:T->step=0;break;}

/** \ingroup TimerTask	
 * 定时任务偿试代码块；此宏后的代码将被组织成独立的分支被执行，
 * 可在代码块内使用TT_AGAIN使该代码块重新被执行， 代码块通常用来检测某一操作结果，如果检测到操作成功标志可
 * 顺利跳转到下一个分支继续执行任务，如果检测不到成功标志可使用TT_AGAIN继续等待并重新检测；如果确认该项操作
 * 失败，也可以使用TT_EXIT以结束整个任务
 * \note 此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象指针
 * \hideinitializer
*/
#define TT_TRY(T) T->counter=0;T->step=__LINE__; case __LINE__:

/** \ingroup TimerTask	
 * 定时任务偿试代码块中用于重新执行偿试代码块
 *
 * \hideinitializer
*/
#define TT_AGAIN return

/** \ingroup TimerTask	
 * 返回定时任务当前分支保持时间(次数),定时任务内部维护一个计数器，当任务通过TT_TRY进入一个代码块时
 * 该计数器清零，并在每个定时周期内自动增一，通过该宏可以获取当前代码块执行的时间(定时周期数)
 *
 * \param T 定时任务对象指针
 * \hideinitializer
*/
#define TT_HOLD_TIME(T) T->counter

/** \ingroup TimerTask	
 * 清零定时任务当前分支保持时间(次数),定时任务内部维护一个计数器，当任务通过TT_TRY进入一个代码块时
 * 该计数器清零，并在每个定时周期内自动增一，通过该宏可手动清零该计数器
 *
 * \param T 定时任务对象指针
 * \hideinitializer
*/
#define TT_HOLD_TIME_CLEAR(T) T->counter=0


/** \ingroup TimerTask	
 * 定时任务中等待某条件　
 * \note 此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象指针
 * \param Y 条件表达式，当Y的结果为0时等待，大于0时退出等待；
 * \hideinitializer
*/
#define TT_WAIT(T,Y) T->counter=0;T->step=__LINE__; case __LINE__:\
										if(!(Y)) return

/** \ingroup TimerTask	
 * 定时任务中延时处理；
 * 注：此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象指针
 * \param Y 延时周期，本函数将实现Y次定时器（本任务绑定的定时器）周期延时；
 * \hideinitializer
*/										
#define TT_DELAY(T,Y) T->counter=0;T->step=__LINE__; case __LINE__:\
										if(T->counter < (Y)) return
										

/** \ingroup TimerTask	
 * 定时任务重新开始执行，重新从TT_BEGIN处开始执行；
 * 
 * \param T 定时任务对象指针
 * \hideinitializer
*/										
#define TT_RESTART(T) {T->step=0,T->counter=0;return;}
	
/** \ingroup TimerTask	
 * 定时任务退出处理　
 * 
 * \param T 定时任务对象指针
 * \hideinitializer
*/				
#define TT_EXIT(T) {frm_end_timer_task(T);return;}

/** \ingroup GenFrame
	\brief 初始化框架，托管应用中无需调用
	\note 需要在main函数开始处调用
*/
void CAFrameInit(void);

/** \ingroup GenFrame
	\brief 框架功能轮循处理，托管应用中无需调用
	\note main函数主循环中调用
*/
void CAFramePoll(void);	
		
#endif
