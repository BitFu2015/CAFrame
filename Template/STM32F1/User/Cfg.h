#ifndef CFG_H
#define CFG_H

#define UINT  uint32_t          //器件无符号整数类型定义

#define	MANAGE_MAIN	1		    		//主程序托管
#define FRM_SKIP_TICK_EN  0     //阻塞时跳过系统计数

#define USER_EVENT_COUNT 3							//通用事件个数

//通用事件ID定义列表
#define EVENT_UART_ID 0		  
#define EVENT_UART2_ID 1
#define EVENT_UART3_ID 2

#endif
