//cfg.h - CAFrame配置文件
#ifndef CFG_H
#define CFG_H

#define	MANAGE_MAIN	0		//主程序托管

//支持定时器个数，在应用程序中定时器编号从0开始小于此值
#define TIMER_MAX_COUNT	4							//定时器个数

#define INPUT_MAX_COUNT 4							//输入口个数

#define EVENT_MAX_COUNT 3							//通用事件个数

//通用事件ID定义列表
#define EVENT_UART1_ID 0		
#define EVENT_UART2_ID 1
#define EVENT_UART3_ID 2

#endif
