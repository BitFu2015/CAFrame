//queue.h

#include "CAFrame.h"

#define EN_INT drv_exit_critical()
#define DS_INT drv_enter_critical()

//从中断向队列插入一字节
void QueueInputFromISR(PHQUEUE Q,uint8_t dat)
{
	if(Q->data_count < Q->buf_size)
	{
		Q->pBuffer[Q->in_index]=dat;		//写入数据
		Q->in_index=(Q->in_index+1) % (Q->buf_size);//调整入口地址
		Q->data_count++;	//调整数据个数(此操作不可被中断)
	}
	else
	{
		if(Q->error<255)
			Q->error++;
	}
} 
//向队列插入一字节
void QueueInput(PHQUEUE Q,uint8_t dat)
{
	DS_INT;
	if(Q->data_count < Q->buf_size)
	{
		Q->pBuffer[Q->in_index]=dat;		//写入数据
		Q->in_index=(Q->in_index+1) % (Q->buf_size);//调整入口地址
		Q->data_count++;	//调整数据个数(此操作不可被中断)
	}
	else
	{
		if(Q->error<255)
			Q->error++;
	}
	EN_INT;
} 
//中断函数中从队列读出一字节
uint8_t QueueOutputFromISR(PHQUEUE Q)
{
	uint8_t Ret=0;
	
	if(Q->data_count > 0)
	{
		Ret=Q->pBuffer[Q->out_index];	//读数据
		
		Q->out_index=(Q->out_index+1) % (Q->buf_size);	//调整出口地址
		Q->data_count--;
	}
	return Ret;
}

//从队列读出一字节
uint8_t QueueOutput(PHQUEUE Q)
{
	uint8_t Ret=0;
	
	DS_INT;	
	if(Q->data_count > 0)
	{
		Ret=Q->pBuffer[Q->out_index];	//读数据
		

		Q->out_index=(Q->out_index+1) % (Q->buf_size);	//调整出口地址
		Q->data_count--;

	}
	EN_INT;	
	return Ret;
}
//获得队列中数据个数
uint16_t QueueGetSize(PHQUEUE Q)
{
	return Q->data_count;
}

//清空队列,执行时不可被中断
void QueueClear(PHQUEUE Q)
{
	DS_INT;
	Q->in_index=0;
	Q->out_index=0;
	Q->data_count=0;
	Q->error=0;
	EN_INT;
}

//初始化一队列
void QueueCreate(PHQUEUE Q,uint8_t *buffer,uint16_t buf_size)
{
	Q->pBuffer=buffer;
	Q->buf_size=buf_size;
	QueueClear(Q);
}
