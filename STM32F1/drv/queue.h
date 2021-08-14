#ifndef QUEUE_H
#define QUEUE_H

//�������ݽṹ
typedef struct QUEUE_S
{
	uint16_t in_index;//��ӵ�ַ
	uint16_t out_index;//���ӵ�ַ
	uint16_t buf_size; //����������
	uint8_t *pBuffer;//����
	volatile uint16_t	data_count; //���������ݸ���
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
