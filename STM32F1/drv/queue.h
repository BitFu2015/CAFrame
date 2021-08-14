#ifndef QUEUE_H
#define QUEUE_H

//队列数据结构
typedef struct QUEUE_S
{
	uint16_t in_index;//入队地址
	uint16_t out_index;//出队地址
	uint16_t buf_size; //缓冲区长度
	uint8_t *pBuffer;//缓冲
	volatile uint16_t	data_count; //队列内数据个数
	uint8_t error;
}HQUEUE,*PHQUEUE;

void QueueInput(PHQUEUE Q,uint8_t dat);
uint8_t QueueOutput(PHQUEUE Q);
void QueueInputFromISR(PHQUEUE Q,uint8_t dat);
uint8_t QueueOutputFromISR(PHQUEUE Q);
uint16_t QueueGetSize(PHQUEUE Q);
void QueueClear(PHQUEUE Q);
void QueueCreate(PHQUEUE Q,uint8_t *buffer,uint16_t buf_size);

#endif
