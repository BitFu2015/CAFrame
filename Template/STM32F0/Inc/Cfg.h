//cfg.h - CAFrame配置文件
#ifndef CFG_H
#define CFG_H

#define UINT  uint32_t          //器件无符号整数类型定义

#define	MANAGE_MAIN	1		    //主程序托管
#define FRM_SKIP_TICK_EN  0     //阻塞时跳过系统计数

#define USER_EVENT_COUNT 1		//用户事件个数

//用户事件ID定义列表,值从0开始递增(0,1,2...)
#define EVENT_UART_ID 0		

#endif
