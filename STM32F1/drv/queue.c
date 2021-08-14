//queue.h

#include "CAFrame.h"

#define EN_INT drv_exit_critical()
#define DS_INT drv_enter_critical()

//���ж�����в���һ�ֽ�
void QueueInputFromISR(PHQUEUE Q,uint8_t dat)
{
	if(Q->data_count < Q->buf_size)
	{
		Q->pBuffer[Q->in_index]=dat;		//д������
		Q->in_index=(Q->in_index+1) % (Q->buf_size);//������ڵ�ַ
		Q->data_count++;	//�������ݸ���(�˲������ɱ��ж�)
	}
	else
	{
		if(Q->error<255)
			Q->error++;
	}
} 
//����в���һ�ֽ�
void QueueInput(PHQUEUE Q,uint8_t dat)
{
	DS_INT;
	if(Q->data_count < Q->buf_size)
	{
		Q->pBuffer[Q->in_index]=dat;		//д������
		Q->in_index=(Q->in_index+1) % (Q->buf_size);//������ڵ�ַ
		Q->data_count++;	//�������ݸ���(�˲������ɱ��ж�)
	}
	else
	{
		if(Q->error<255)
			Q->error++;
	}
	EN_INT;
} 
//�жϺ����дӶ��ж���һ�ֽ�
uint8_t QueueOutputFromISR(PHQUEUE Q)
{
	uint8_t Ret=0;
	
	if(Q->data_count > 0)
	{
		Ret=Q->pBuffer[Q->out_index];	//������
		
		Q->out_index=(Q->out_index+1) % (Q->buf_size);	//�������ڵ�ַ
		Q->data_count--;
	}
	return Ret;
}

//�Ӷ��ж���һ�ֽ�
uint8_t QueueOutput(PHQUEUE Q)
{
	uint8_t Ret=0;
	
	DS_INT;	
	if(Q->data_count > 0)
	{
		Ret=Q->pBuffer[Q->out_index];	//������
		

		Q->out_index=(Q->out_index+1) % (Q->buf_size);	//�������ڵ�ַ
		Q->data_count--;

	}
	EN_INT;	
	return Ret;
}
//��ö��������ݸ���
uint16_t QueueGetSize(PHQUEUE Q)
{
	return Q->data_count;
}

//��ն���,ִ��ʱ���ɱ��ж�
void QueueClear(PHQUEUE Q)
{
	DS_INT;
	Q->in_index=0;
	Q->out_index=0;
	Q->data_count=0;
	Q->error=0;
	EN_INT;
}

//��ʼ��һ����
void QueueCreate(PHQUEUE Q,uint8_t *buffer,uint16_t buf_size)
{
	Q->pBuffer=buffer;
	Q->buf_size=buf_size;
	QueueClear(Q);
}
