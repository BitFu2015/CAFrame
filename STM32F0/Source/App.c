/**************************************************
	�ļ�����App.c
	��  �ܣ�CAFrame ��ʱ����Ӧ��ʾ�� 
	MCU:STM32F030C8T6
	ʱ��:48MHz(���8MHz����)
	��  ����V1.0
	
	о�������(http://www.chipart.cn)  ��Ȩ����
	������ҳ:http://www.chipart.cn/projects/prj_hit.asp?id=8

	
	��ʷ��¼ 
	о�� V1.0 2015-08-12  ����
**************************************************/ 
#include "CAFrame.h"

/*��˵����
��ʱ��������ʵ����ʱ�ֲ��������ܡ�
����������Ҫʵ������һ�����ܣ���һ���¿�ϵͳ�У�����Ȱ���ǰ�ȴ򿪷������ʱ10���򿪵��ȣ�
�ر�ʱ�ȹرյ���Ȱ�������ʱ30��ط��������Ҫ��رպ���3�����ڲ����ٴδ򿪡�
��ʾ��ʹ��CAFrame��ʱ����ʵ���˴˹��ܣ�
*/

//��ʱ���������
TIMER_TASK g_OpenTask,g_CloseTask;

uint8_t g_RunStatus=0;			//��ǰ����״̬
uint8_t g_ControlStatus=0;	//��ǰ��������״̬

void Timer1Event(void)
{
	LED_TOGGLE; 

	printf("RunStatus:%d,ControlStatus:%d\r\n",g_RunStatus,g_ControlStatus);
}

//������������
void OpenTask(PTIMER_TASK pTask)
{
	if(pTask->step == 0)
	{
		drv_output(IO_Y1,1);//�򿪷��
		frm_timer_task_nextstep(pTask); //�л�����һ��
	}
	else if(pTask->step == 1)
	{
		if(pTask->counter >= 10) //��ʱ10�� , pTask->counter ��ʾ��ǰ״̬�ѱ��ֵĶ�ʱʱ��
		{
			drv_output(IO_Y2,1);//�򿪵���
			frm_end_timer_task(pTask); //ֹͣ�ö�ʱ����
			g_RunStatus=1;//�������
		}
	}
	else;
}

//�ػ���������
void CloseTask(PTIMER_TASK pTask)
{
	if(pTask->step == 0)
	{
		drv_output(IO_Y2,0);//�رյ���
		frm_timer_task_nextstep(pTask);
	}
	else if(pTask->step == 1)
	{
		if(pTask->counter > 30) //�ȴ�30��رշ��
		{
			drv_output(IO_Y1,0);//�رշ��
			frm_timer_task_nextstep(pTask);
		}
	}
	else if(pTask->step == 2)
	{
		if(pTask->counter > 180) //�ȴ�3���ӱ���Ƶ������
		{
			frm_end_timer_task(pTask);
			g_RunStatus=0;//�ػ����
		}
	}
	else ;
}

//��ʱ��1�¼�������
void TaskTimerEvent(void)
{
	//ע�������ʱ������Ķ�ʱ������ִ��״̬�������´��벻��õ�ִ��
	//ֻ�����а���˶�ʱ���Ķ�ʱ���������ֹͣʱ�Ż�ִ�д˺���
	
	//�������״̬�ı�
	if(g_ControlStatus != g_RunStatus)
	{
		if(g_RunStatus)//�����ڿ���״̬
			frm_begin_timer_task(&g_CloseTask);
		else
			frm_begin_timer_task(&g_OpenTask);
	}
}

void InputEvent1(uint16_t x)
{
	//���ƿ���״̬�л�
	g_ControlStatus=g_ControlStatus?0:1; 
}

/*Ӧ���������ʼ������
�ڿ�ܳ���(frame.c)�е��õ�ΨһӦ�ýӿں���
������ҪΪ��ܳ���װ������Ҫ���¼�������*/
void InitApp(void)
{
	//��ͨ��ʱ�¼�
	frm_install_timer_event(0,2000,Timer1Event);	 	//���Ʋ���ʱ��
	
	//���ƶ�ʱ��
	frm_install_timer_event(1,1000,TaskTimerEvent);
  frm_bind_timer_task(1,&g_OpenTask,OpenTask); //���������
  frm_bind_timer_task(1,&g_CloseTask,CloseTask);//��ػ�����
	
	//��װ���ư��������¼�
	//IO_X1�����½��ش����¼�,�͵�ƽ���ٱ���5ms
	frm_install_input_event(IO_X1,InputEvent1,5,ET_FALLING_EDGE);
}


