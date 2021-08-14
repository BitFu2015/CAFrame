/*********************************************
	�ļ�����Frame.c
	��  �ܣ�CAFrameӦ�ó����� 
	��  ����V2.2.4
	
	о�������(http://www.chipart.cn)  ��Ȩ����
	������ҳ:http://www.chipart.cn/projects/prj_hit.asp?id=8
	
	��ʷ��¼
	о�� V1.0.0 2012-05-27  ����
	о�� V2.0.0 2013-10-08	�淶��ͨ���¼�����ʱ�����Ӹ�λ����
	о�� V2.0.1 2013-10-09  ���Ӷ�ʱ������
	о�� V2.0.2 2013-12-15	�淶��ͨ���¼�
	о�� V2.1.0 2015-07-24  ���ƶ�ʱ�����ܣ��ɰ󶨵���ʱ��ʹ��(�Ƽ�ʹ�÷�ʽ)
	о�� V2.2.0 2015-11-05	Ϊ��ʱ����������һ�׽ṹ���ƺ�
	о�� V2.2.1 2017-06-25	����������(MAIN)�й�ѡ��
	о�� V2.2.2 2017-10-07	�����¼���֮ǰ��ƽ״̬����ʱ�����������
	о�� V2.2.3 2019-05-07  ͨ���¼�����ʱ�����Ƿ�װ����������⵽��־������
	о�� V2.2.4 2021-07-27  ��ʱ����ִ�к��ߵĶ�ʱ���ݲ�ִ��������
*********************************************/
#include "CAFrame.h"

volatile uint8_t g_TimerFlag;//1ms��ʱ���

//ͨ���¼�����
static EVENT g_GenEvent[EVENT_MAX_COUNT];

//�����¼�����
static EVENT_TIMER g_TimerEvent[TIMER_MAX_COUNT];//��ʱ���¼�
static EVENT_INPUT g_InputEvent[INPUT_MAX_COUNT];//�����¼�

void InitApp(void);	//Ӧ�ó����ʼ��

void frm_init(void)
{
	uint8_t i;
	
	for(i=0;i<TIMER_MAX_COUNT;i++)
		g_TimerEvent[i].id=0xff;
	
	for(i=0;i<INPUT_MAX_COUNT;i++)
		g_InputEvent[i].port=0xff;
		
	for(i=0;i<EVENT_MAX_COUNT;i++)
	{
		g_GenEvent[i].flag=0;
		g_GenEvent[i].Func=0;
	}
}

//���ܷ���1ms��ʱ�ź�
void frm_inc_timer_tick(void)
{
	g_TimerFlag++;	
}

//����ͨ���¼�����driver.c�е���
void frm_set_event(uint8_t event_id)
{
	if(event_id<EVENT_MAX_COUNT)
		g_GenEvent[event_id].flag=1;
}

//��ͨ���¼����,��driver.c�е���
/*
void frm_clr_event(uint8_t event_id)
{
	if(event_id<EVENT_MAX_COUNT)
		g_GenEvent[event_id].flag=0;
}*/

uint8_t frm_get_event_state(uint8_t event_id)
{
	if(event_id<EVENT_MAX_COUNT)
		return g_GenEvent[event_id].flag;
	return 1;//����
}

//�����¼���������
void frm_input_trigger(void)
{
	uint8_t i,tmp;
	
	for(i=0;i<INPUT_MAX_COUNT;i++)
	{
		if(g_InputEvent[i].port !=i)  //δ��װ�¼���δ��ʼ��
			continue;
			
		tmp=drv_input(i);
		
		if(tmp==g_InputEvent[i].state)
		{
			if(g_InputEvent[i].filter_counter == 0) //����״̬
			{
				if(g_InputEvent[i].hold < 0xFFFF)
					g_InputEvent[i].hold++;					
				continue; 
			}
			
			g_InputEvent[i].filter_counter--;
			if(g_InputEvent[i].filter_counter==0)//���뱣��״̬
			{
				if((g_InputEvent[i].type == 2)||(g_InputEvent[i].type==tmp))
				{
					g_InputEvent[i].Func(g_InputEvent[i].hold);
				}
				g_InputEvent[i].hold=g_InputEvent[i].filter_config;
			}
		}
		else
		{
			g_InputEvent[i].state=tmp;
			g_InputEvent[i].filter_counter=g_InputEvent[i].filter_config;
		}
	}	
}

//��ȡ�˲�������״̬
uint8_t frm_input(uint8_t port)
{
	return g_InputEvent[port].state;
}

//��ʱ���¼���������
void frm_timer_trigger(void)
{
	uint8_t i,flag;
	PTIMER_TASK pTask;

	for(i=0;i<TIMER_MAX_COUNT;i++)
	{
		if(g_TimerEvent[i].id!=i)//δʹ��
			continue;

		if(++g_TimerEvent[i].cnt < g_TimerEvent[i].cfg)
			continue;
	  
	  //��ʱ����������	
		pTask=g_TimerEvent[i].pTaskList;
		flag=0;
		while(pTask)
		{
		  if(pTask->busy)//�������ڴ���
		  {
		  	pTask->counter++; //��������һ
		    if(pTask->Func) //����д�����
		      pTask->Func(pTask);
		    flag=1;
		  }
		  pTask=(PTIMER_TASK)pTask->next;
		}
		
		if((flag==0) && (g_TimerEvent[i].Func != 0)) //���û�����񱻴���
	  	g_TimerEvent[i].Func();
		
		g_TimerEvent[i].cnt=0;
	}//for
}

void frm_event_trigger(void)
{
	uint8_t i;
	for(i=0;i<EVENT_MAX_COUNT;i++)
	{
		if(g_GenEvent[i].flag)//�¼����
		{
			if(g_GenEvent[i].Func!=0)
				g_GenEvent[i].Func();
			g_GenEvent[i].flag=0;
		}	
	}
}

void CAFrameInit(void)
{	
	drv_init();//������ʼ��
	frm_init();//��ܳ�ʼ��
	InitApp();//Ӧ�ó�ʼ��
}

void CAFramePoll(void)
{		
	//1ms��ʱ�����¼�
	if(g_TimerFlag)
	{
		frm_timer_trigger();
		frm_input_trigger();
		
		drv_enter_critical();
		g_TimerFlag--;
		drv_exit_critical();
		
		drv_wdt_reset(); 
	}
	
	//ͨ���¼�����
	frm_event_trigger();
}

#if MANAGE_MAIN > 0
int main(void)     
{
	CAFrameInit();
	while(1)
	{
		CAFramePoll();
	}//mail loop
}
#endif

//����˿ڳ�ʼ��
void frm_install_input_event(uint8_t port,void (*fn)(uint16_t),uint8_t filter,uint8_t type)
{
	if(port >= INPUT_MAX_COUNT)
		return ;
		
	g_InputEvent[port].port=port;
	g_InputEvent[port].filter_config=filter;
	g_InputEvent[port].type=type;
	g_InputEvent[port].Func=fn;
	g_InputEvent[port].filter_counter=0;
	g_InputEvent[port].state=drv_input(port);
}

//��ʱ����ʼ��
void frm_install_timer_event(uint8_t id,uint16_t tms,void (*fn)(void))
{
	if(id>=TIMER_MAX_COUNT)
		return ;
		
	g_TimerEvent[id].id=id;
	g_TimerEvent[id].cfg=tms;
	g_TimerEvent[id].Func=fn;
	g_TimerEvent[id].cnt=0;
	g_TimerEvent[id].pTaskList=0;
}

void frm_install_event(uint8_t id,void (*fn)(void))
{
	if(id>=EVENT_MAX_COUNT)
		return ;
	g_GenEvent[id].Func=fn;
}

//��ʱ��ʹ��
void frm_timer_enable(uint8_t id)
{
	if(id<TIMER_MAX_COUNT)
		g_TimerEvent[id].id=id;
}

//��ʱ������
void frm_timer_disable(uint8_t id)
{
	if(id < TIMER_MAX_COUNT)
		g_TimerEvent[id].id=0xff;
}

//��ʱ�����¿�ʼ��������λ��
void frm_timer_reset(uint8_t id)
{
	if(id < TIMER_MAX_COUNT)
		g_TimerEvent[id].cnt=0;
}

//��ʼ��ʱ������
void frm_begin_timer_task(PTIMER_TASK pTask)
{
	if(pTask->busy==0)
	{
		pTask->busy=1;
		pTask->step=0;
		pTask->counter=0;
	}
}

//�رն�ʱ������
void frm_end_timer_task(PTIMER_TASK pTask)
{
  pTask->busy=0;
}

//����󶨵���ʱ��,fnΪ��ʱ��������
void frm_bind_timer_task(uint8_t id,PTIMER_TASK pTask,void (*fn)(PTIMER_TASK))
{
  PTIMER_TASK p;
  
  pTask->busy=0; //Ĭ�Ϲر�����
  pTask->Func=fn;
  pTask->next=0;
  
  //����б�Ϊ��
  if(g_TimerEvent[id].pTaskList==0)
  {
    g_TimerEvent[id].pTaskList=pTask;
    return ;
  }

  //�ҵ����һ��TIMER_TASK
  p=g_TimerEvent[id].pTaskList;
	while(p->next)
	  p=p->next;
	
	p->next=pTask;  
}






