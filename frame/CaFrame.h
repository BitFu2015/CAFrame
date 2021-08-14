#ifndef CAFRAME_H
#define CAFRAME_H

#include "Driver.h"
#include "cfg.h"

#define USE_CAFRAME		//CAFrame应用标志

/** \mainpage CAFrame单片机C语言程序开发框架
 
 \author 芯艺工作室 - http://www.chipart.cn
 
 \par 简介 
	CAFrame 单片机C语言程序开发框架可用于不同的单片机，尤其适合8位小系统的软件开发。框架将硬件驱动和应用逻辑的实现分开，
	以事件驱动的方式编写应用程序，从而使得应用功能的编写就如同VB6程序那样简单易维护。
	框架程序共由三个头文件（CAFrame.h Cfg.h Driver.h）和三个源文件(Driver.c Frame.c App.c)组成
	其层次结构如图： \n
 	<img src=layer.jpg> \n
 	Driver.c中的代码用于处理与硬件相关的底层驱动，Driver.h文件中声明与硬件相关的一些符号定义和用户自定义驱动函数的声明 \n
 	驱动程序的编写主要通过修改这两个文件来实现. \n \n
	CAFrame.h是框架标准符号、结构、函数的声明处，Frame.c是框架的实现文件，无论任何时候都不应该修改这两个文件。 \n	
 	注意： CAFrame.h文件中声明的以[drv_]为前缀的函数是框架所必须的驱动接口函数，这些函数在driver.c中必须实现. \n \n
	App.c是应用逻辑实现文件，文件需要实现一个固定的函数InitApp,此函数在框架初始化后第一个被调用，在这个函数中设置好所有需要的事件处理函数。作为一个完整的app.c文件，下面的示例说明了开发者需要做些什么
 	\code
 	#include "CAFrame.h"
 	void TimerEvent(void)//定时器事件处理函数
 	{
 		LED_FLASH;
 	}
 	void InitApp(void)
 	{
 		frm_install_timer_event(0,200,TimerEvent);//安装0号定时器事件处理函数,定时触发周期为200ms
 	}
 	\endcode
	
	\par 程序运行优先级
	CAFrame并非是一个操作系统,而是一个典型的前后台程序结构，程序运行优先级只有中断级（INT_LEVEL）和应用级（APP_LEVEL).
	driver.c中一部分硬件驱动程序在中断中调用,它们工作在INT_LEVEL级,但这些对于APP来说是透明的，无需考虑的，
	所有的接口函数以及用户代码（app.c)均运行在同一个优先级（APP_LEVEL），为此用户程序不会相互中断。
	
	\par CAFrame配置	
	CAFrame的配置十分简单，Cfg.h文件中TIMER_MAX_COUNT是指定在APP中用到的最大定时器个数，INPUT_MAX_COUNT是系统输入口个数
	EVENT_MAX_COUNT为通用事件ID个数。下面的“通用事件ID定义列表中”列出所用到的所有通用事件ID，值从0递增，但要保证小于EVENT_MAX_COUNT.
	\code
	
	#define	MANAGE_MAIN	1		//主程序托管
	
	//支持定时器个数，在应用程序中定时器编号从0开始小于此值
	#define TIMER_MAX_COUNT	4							//定时器个数
	
	#define INPUT_MAX_COUNT 2							//输入口个数
	
	#define EVENT_MAX_COUNT 2							//通用事件个数
	
	//通用事件ID定义列表
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
	
	\par 通用事件应用
	事件是框架中驱动层与应用层同步的主要方式，框架支持三种事件，分别是定时器事件、输入事件和通用事件，定时器事件和输入事件是框架固定的
	特殊事件，而通用事件是用户编写外设驱动与应用层同步的主要方式，如UART,ADC的中断中可以通过通用事件的方式在应用层触发事件。通用事件运行原理如图： \n 
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
  */
  

/** \defgroup InputDriver 开关输入接口

	开关输入处理方式有两种
	\par 事件驱动方式
	
	示例如下(app.c)：
	\code
	void InputEvent1(uint16_t x)
	{
		//x为事件产生前的电平状态保持时间
		LED_FLASH;
	}	 
	void InitApp(void)
	{
		frm_install_input_event(IO_X1,InputEvent1,5,ET_ALL_EDGE);//1号输入口状态发生改变时调用InputEvent1函数
		// ... 其它事件函数的安装
	}
	\endcode
	
	\par 定时器内扫描方式
	
	在定时器内通过drv_input或frm_input函数检测输入状态，并作出相应的操作;
	\note 如果使用frm_input函数，则必须在InitApp中将相应端口输入事件初始化成ET_DISABLE模式,如：
	
	\code					
	frm_install_input_event(IO_X1,0,5,ET_DISABLE); 
	\endcode
*/

/** \defgroup OutputDriver 输出控制接口
	输出控制函数包含开关量输出接口函数drv_output和模拟量输出接口函数drv_dac_output.
*/
/** \defgroup TimerDriver  定时器接口

	定时器操作十分简单，一个最简单的示例如下(app.c)：
	\code
	void TimerEvent1(void)
	{
		LED_FLASH;//发光管闪烁
	}	
	void InitApp(void)
	{
		frm_install_timer_event(0,300,TimerEvent1);//安装0号定时器事件处理函数,定时触发周期为300ms
		// ... 其它初始化操作
	}
	\endcode
	\note 要注意frm_install_timer_event函数第一个参数要小于在Cfg.h中定义的TIMER_MAX_COUNT
*/

/** \defgroup GenDriver 标准驱动函数

	驱动通用接口函数 为了使驱动层和应用层接口程序标准化，框架定义了一些标准化的函数，
	框架移植到新的硬件上时应在driver.c中实现这些函数功能。
*/

/** \defgroup GenFrame 框架事件触发接口

	在驱动中触发框架用的接口函数
*/

/** \defgroup TimerTask 定时任务管理
	定时任务用于实现延时分步操作功能。
	例如我们需要实现这样一个功能：在一个温控系统中，电加热棒打开前先打开风机再延时10秒后打开电热，
	关闭时先关闭电加热棒，再延时30秒关风机，并且要求关闭后在3分钟内不得再次打开。
	使用定时任务实现此功能的代码如下：
	\code
	#include "CAFrame.h"

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
		//普通定时事件，此定时器与此功能无关，仅用于调试
		frm_install_timer_event(0,2000,Timer1Event);
		
		//控制定时器
		frm_install_timer_event(1,1000,TaskTimerEvent);
		frm_bind_timer_task(1,&g_OpenTask,OpenTask); //邦定开机任务
		frm_bind_timer_task(1,&g_CloseTask,CloseTask);//邦定关机任务
		
		//安装控制按键输入事件
		//IO_X1输入下降沿触发事件,低电平至少保持5ms
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
	uint16_t step; 	//当前执行步骤
	uint16_t counter; //延时用计数器
	
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
}EVENT_TIMER,*PEVENT_TIMER;

/*
	通用事件管理数据结构
*/
typedef struct tag_event
{
	//事件发生标记
	uint8_t flag;
	//事件处理函数
	void (*Func)(void);
}EVENT,*PEVENT;

/** \ingroup InputDriver
	\brief 读取开关输入端口状态，用于读取滤波后的开关输入状态
	\param port 端口:IO_X1 ~ IO_X6
	\return 有信号返回1  无信号返回0
*/
uint8_t frm_input(uint8_t port);

/** \ingroup InputDriver
	\brief 安装开关输入事件
	\param port 端口号:IO_X1 ~ IO_X6
	\param fn 事件处理函数，原型: void func(uint16_t t)<br>参数t为之前电平状态保持的时间(ms)，如果保持时间大于0xFFFF则t返回0xFFFF;
	\param filter 滤波计数器，以毫秒为单位的信号确认时间
	\param type ET_FALLING_EDGE：下降沿触发<BR>  ET_RISING_EDGE：上升沿触发 <BR>ET_ALL_EDGE：边沿触发<BR>ET_DISABLE：事件触发禁止(检测应用时使用)
*/
void frm_install_input_event(uint8_t port,void (*fn)(uint16_t),uint8_t filter,uint8_t type);

/** \ingroup TimerDriver
	\brief 安装定时器函数
	\param id 定时器编号 0开始计数的编号
	\param tms 定时器事件产生周期(毫秒为单位)
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
	\brief 安装通用事件
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
	\brief 触发通用事件，此函数通常在驱动的中断中调用，用于实现对应事件的触发
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

/** \ingroup GenDriver
	\brief 安全读取EEPROM接口函数,在驱动(driver.c)中必须实现此函数。
	\param addr 读取地址
	\param buf 读取缓冲区
	\param size 读取长度(字节)
*/
void drv_eeprom_read(uint16_t addr,uint8_t *buf,uint8_t size);

/** \ingroup GenDriver
	\brief 安全写入EEPROM接口函数,在驱动(driver.c)中必须实现此函数。
	\param addr 写入地址
	\param buf 写入缓冲区
	\param size 写入长度(字节)
*/
void drv_eeprom_write(uint16_t addr,uint8_t *buf,uint8_t size);

/** \ingroup TimerTask
	\brief 开始执行定时任务
	\param pTask 定时任务对象地址
*/
void frm_begin_timer_task(PTIMER_TASK pTask);

/** \ingroup TimerTask
	\brief 停止指定的定时任务
	\param pTask 定时任务对象地址
*/
void frm_end_timer_task(PTIMER_TASK pTask);

/** \ingroup TimerTask	
 * 设置定时任务的当前执行步骤
 * 
 * \note 此函数为兼容老版程序而保留，新设计中应避免使用！　
 * \hideinitializer
*/
#define frm_timer_task_setstep(X,Y) (X->step=Y,X->counter=0)

/** \ingroup TimerTask	
 * 设置定时任务转换到下一个执行步骤　
 * 
 * \note 此函数为兼容老版程序而保留，新设计中应避免使用！
 * \hideinitializer
*/
#define frm_timer_task_nextstep(X)  (X->step++,X->counter=0)

/** \ingroup TimerTask
	\brief 定时任务绑定到定时器
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
 * \note 此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象地址
 * \hideinitializer
*/	
#define TT_END(T) default:frm_end_timer_task(T);break;}

/** \ingroup TimerTask	
 * 定时任务循环处理,在定时任务的结束处调用；用此宏结束的定时任务顺序执行完成后自动重新回到开始处继续执行，
 * 不会自动结束；
 * \note 此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象地址
 * \hideinitializer
*/
#define TT_LOOP(T) default:T->step=0;T->counter=0;break;}

/** \ingroup TimerTask	
 * 定时任务中等待某条件　
 * \note 此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象地址
 * \param Y 条件表达式，当Y的结果为0时等待，大于0时退出等待；
 * \hideinitializer
*/
#define TT_WAIT(T,Y) T->step=__LINE__; case __LINE__:\
										if(!(Y)) return;\
										T->counter=0

/** \ingroup TimerTask	
 * 定时任务中延时处理；
 * 注：此函数为任务分支结构处理函数，不能在条件、循环等结构语句内使用！
 * 
 * \param T 定时任务对象地址
 * \param Y 延时周期，本函数将实现Y次定时器（本任务绑定的定时器）周期延时；
 * \hideinitializer
*/										
#define TT_DELAY(T,Y) T->step=__LINE__; case __LINE__:\
										if(T->counter < (Y)) return ;\
										T->counter=0 

/** \ingroup TimerTask	
 * 定时任务重新开始执行，重新从TT_BEGIN处开始执行；
 * 
 * \param T 定时任务对象地址
 * \hideinitializer
*/										
#define TT_RESTART(T) T->step=0,T->counter=0;return 
	
/** \ingroup TimerTask	
 * 定时任务退出处理　
 * 
 * \param T 定时任务对象地址
 * \hideinitializer
*/				
#define TT_EXIT(T) frm_end_timer_task(T);return 

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
