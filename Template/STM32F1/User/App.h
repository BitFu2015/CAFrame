//APP.H
#ifndef __APP_H
#define __APP_H

#define DEBUG_MODE 0

//配置参数列表
#define CFG_ITEM_COUNT 13 //以下结构体中变量个数
typedef struct
{
	uint16_t cfg_t;//解冻温度
	uint16_t cfg_hc;//温度控制回差

	uint16_t cfg_r;//解冻湿度
	uint16_t cfg_js_on_time;	//加湿打开时间
	uint16_t cfg_js_off_time;//加湿关闭时间

	uint16_t cfg_dc_on_time;//电场打开时间
	uint16_t cfg_dc_off_time;//电场关闭时间

	uint16_t cfg_mc_on_time;//脉冲打开时间
	uint16_t cfg_mc_off_time;//脉冲关闭时间

	uint16_t cfg_zw_on_time;//紫外打开时间
	uint16_t cfg_zw_off_time;//紫外关闭时间	
	
	uint16_t cfg_cs_zq;//除霜周期
	uint16_t cfg_cs_sj;//除霜时间
}cfg_items_t;

typedef union
{
	uint16_t arr[CFG_ITEM_COUNT];
	cfg_items_t items;
}cfg_list_t;

//参数条目描述表
typedef struct
{
	char *name;
	char *unit;
	int16_t max_val;
	int16_t min_val;
	uint8_t type;	;//0:整数 1:温度
}CFG_ITEM_DESC,*PCFG_ITEM_DESC;

#define TYPE_INT	0
#define TYPE_TEMP 1

//运行参数
typedef struct tag_RunPar
{
	uint8_t 	Mode; 		//当前运行模式 0:关机 1:解冻
	uint16_t  RunMode;	//工作模式 0:待机 1:制热 2:制冷 3:除霜
	uint8_t RunTime[3];	//已运行时间：小时，分钟，CRC	
	
	uint16_t T[5];      //当前测量温度 0:am2301测量温度 1:柜内温度 2:冷凝温度 3:蒸发温度 4:环境温度 5:保留
	uint16_t R;					//当前湿度
	
	//配置参数
	uint16_t cfg_js_en;//加湿控制使能
	uint16_t cfg_mc_en;//脉冲控制使能	
	uint16_t cfg_dc_en;//电场控制使能
	uint16_t cfg_zw_en;	//紫外控制使能
	cfg_list_t ConfigList;
	
	uint16_t RegCounter;//注册计数器
	uint16_t RegEnable; //注册码使能标志	
	
	uint8_t Alarm;//报警状态
	uint8_t AlarmUpdateDelay;//更新显示报警延时计数器
}RUN_PAR,*PRUN_PAR;


PRUN_PAR GetRunPar(void);

uint16_t I2T(int n);
int16_t T2I(uint16_t t);

#define ALARM_EMPTY 0			//无报警
#define ALARM_T3		1			//冷凝温度传感器故障
#define ALARM_DOOR	2			//门开报警
#define ALARM_R			3			//湿度传感器报警

#define ALARM_WARNING 10	//大于此值为停机报警

#define ALARM_T1		11		//柜内温度传感器故障
#define ALARM_T2		12		//蒸发温度传感器故障
#define ALARM_LN		13		//冷凝温度报警
#define ALARM_USER	14		//紧急停机
#define ALARM_REG		15		//软件授权故障
void AlarmSet(uint8_t alarm);
void AlarmClr(uint8_t alarm);
void AlarmGetName(uint8_t alarm,char *alm_str);


//配置读写接口
uint16_t LoadConfigU16(uint16_t addr);
int16_t LoadConfigI16(uint16_t addr);
void SaveConfigU16(uint16_t addr,uint16_t val);
void SaveConfigItem(uint16_t item_index);

//图标更新接口
void InitFlag(void);
void UpdateFlag(void);
void SetFlag(uint8_t index,uint8_t flg);

#include "wnd.h"
#include "display.h"
#include "hmi_driver.h"
#include "control.h"

#endif
