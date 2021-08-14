#ifndef CAFRAME_H
#define CAFRAME_H

#include "Driver.h"
#include "cfg.h"

#define USE_CAFRAME		//CAFrameӦ�ñ�־

/** \mainpage CAFrame��Ƭ��C���Գ��򿪷����
 
 \author о�չ����� - http://www.chipart.cn
 
 \par ��� 
	CAFrame ��Ƭ��C���Գ��򿪷���ܿ����ڲ�ͬ�ĵ�Ƭ���������ʺ�8λСϵͳ�������������ܽ�Ӳ��������Ӧ���߼���ʵ�ַֿ���
	���¼������ķ�ʽ��дӦ�ó��򣬴Ӷ�ʹ��Ӧ�ù��ܵı�д����ͬVB6������������ά����
	��ܳ���������ͷ�ļ���CAFrame.h Cfg.h Driver.h��������Դ�ļ�(Driver.c Frame.c App.c)���
	���νṹ��ͼ�� \n
 	<img src=layer.jpg> \n
 	Driver.c�еĴ������ڴ�����Ӳ����صĵײ�������Driver.h�ļ���������Ӳ����ص�һЩ���Ŷ�����û��Զ����������������� \n
 	��������ı�д��Ҫͨ���޸��������ļ���ʵ��. \n \n
	CAFrame.h�ǿ�ܱ�׼���š��ṹ����������������Frame.c�ǿ�ܵ�ʵ���ļ��������κ�ʱ�򶼲�Ӧ���޸��������ļ��� \n	
 	ע�⣺ CAFrame.h�ļ�����������[drv_]Ϊǰ׺�ĺ����ǿ��������������ӿں�������Щ������driver.c�б���ʵ��. \n \n
	App.c��Ӧ���߼�ʵ���ļ����ļ���Ҫʵ��һ���̶��ĺ���InitApp,�˺����ڿ�ܳ�ʼ�����һ�������ã���������������ú�������Ҫ���¼�����������Ϊһ��������app.c�ļ��������ʾ��˵���˿�������Ҫ��Щʲô
 	\code
 	#include "CAFrame.h"
 	void TimerEvent(void)//��ʱ���¼�������
 	{
 		LED_FLASH;
 	}
 	void InitApp(void)
 	{
 		frm_install_timer_event(0,200,TimerEvent);//��װ0�Ŷ�ʱ���¼�������,��ʱ��������Ϊ200ms
 	}
 	\endcode
	
	\par �����������ȼ�
	CAFrame������һ������ϵͳ,����һ�����͵�ǰ��̨����ṹ�������������ȼ�ֻ���жϼ���INT_LEVEL����Ӧ�ü���APP_LEVEL).
	driver.c��һ����Ӳ�������������ж��е���,���ǹ�����INT_LEVEL��,����Щ����APP��˵��͸���ģ����迼�ǵģ�
	���еĽӿں����Լ��û����루app.c)��������ͬһ�����ȼ���APP_LEVEL����Ϊ���û����򲻻��໥�жϡ�
	
	\par CAFrame����	
	CAFrame������ʮ�ּ򵥣�Cfg.h�ļ���TIMER_MAX_COUNT��ָ����APP���õ������ʱ��������INPUT_MAX_COUNT��ϵͳ����ڸ���
	EVENT_MAX_COUNTΪͨ���¼�ID����������ġ�ͨ���¼�ID�����б��С��г����õ�������ͨ���¼�ID��ֵ��0��������Ҫ��֤С��EVENT_MAX_COUNT.
	\code
	
	#define	MANAGE_MAIN	1		//�������й�
	
	//֧�ֶ�ʱ����������Ӧ�ó����ж�ʱ����Ŵ�0��ʼС�ڴ�ֵ
	#define TIMER_MAX_COUNT	4							//��ʱ������
	
	#define INPUT_MAX_COUNT 2							//����ڸ���
	
	#define EVENT_MAX_COUNT 2							//ͨ���¼�����
	
	//ͨ���¼�ID�����б�
	#define EVENT_ADC_ID 0		
	#define EVENT_UART_ID 1
	\endcode
	MANAGE_MAIN ��ֵ����������Ŀ��ں���(main)�Ƿ��йܸ���ܣ����ֵ��1���͵����û�����ʵ��main��������main�����йܸ��˿�ܡ�
	���MANAGE_MAIN����Ϊ0,��Ϊ���й�Ӧ�ã���Ҫ���û�������ʵ��һ�����µ�main����:
	\code
	int main(void)
	{
		CAFrameInit();//��ܳ�ʼ��
		
		while(1)
		{
			//..   //��ѭ���е���������
			
			CAFramePoll();	//���ѭ��
		}
		return 0;
	}	
	\endcode
	
	\par ͨ���¼�Ӧ��
	�¼��ǿ������������Ӧ�ò�ͬ������Ҫ��ʽ�����֧�������¼����ֱ��Ƕ�ʱ���¼��������¼���ͨ���¼�����ʱ���¼��������¼��ǿ�̶ܹ���
	�����¼�����ͨ���¼����û���д����������Ӧ�ò�ͬ������Ҫ��ʽ����UART,ADC���ж��п���ͨ��ͨ���¼��ķ�ʽ��Ӧ�ò㴥���¼���ͨ���¼�����ԭ����ͼ�� \n 
	<img src=event.jpg> \n
	������Ҫ�������ļ�(cfg.h)�ж����¼�ID,��
	\code 
	#define EVENT_UART_ID 1
	\endcode
	Ȼ���������ж��д����¼���
	\code
	ISR
	{
		...
		frm_set_event(EVENT_UART_ID);
		...
	}
	\endcode
	�����InitApp�������Ѿ���frm_install_event��װ���¼�����������ô�ж���һ��������¼�����ܻ����жϽ������ں�̨��APP_LEVEL��������
	�¼���������
  */
  

/** \defgroup InputDriver ��������ӿ�

	�������봦��ʽ������
	\par �¼�������ʽ
	
	ʾ������(app.c)��
	\code
	void InputEvent1(uint16_t x)
	{
		//xΪ�¼�����ǰ�ĵ�ƽ״̬����ʱ��
		LED_FLASH;
	}	 
	void InitApp(void)
	{
		frm_install_input_event(IO_X1,InputEvent1,5,ET_ALL_EDGE);//1�������״̬�����ı�ʱ����InputEvent1����
		// ... �����¼������İ�װ
	}
	\endcode
	
	\par ��ʱ����ɨ�跽ʽ
	
	�ڶ�ʱ����ͨ��drv_input��frm_input�����������״̬����������Ӧ�Ĳ���;
	\note ���ʹ��frm_input�������������InitApp�н���Ӧ�˿������¼���ʼ����ET_DISABLEģʽ,�磺
	
	\code					
	frm_install_input_event(IO_X1,0,5,ET_DISABLE); 
	\endcode
*/

/** \defgroup OutputDriver ������ƽӿ�
	������ƺ�����������������ӿں���drv_output��ģ��������ӿں���drv_dac_output.
*/
/** \defgroup TimerDriver  ��ʱ���ӿ�

	��ʱ������ʮ�ּ򵥣�һ����򵥵�ʾ������(app.c)��
	\code
	void TimerEvent1(void)
	{
		LED_FLASH;//�������˸
	}	
	void InitApp(void)
	{
		frm_install_timer_event(0,300,TimerEvent1);//��װ0�Ŷ�ʱ���¼�������,��ʱ��������Ϊ300ms
		// ... ������ʼ������
	}
	\endcode
	\note Ҫע��frm_install_timer_event������һ������ҪС����Cfg.h�ж����TIMER_MAX_COUNT
*/

/** \defgroup GenDriver ��׼��������

	����ͨ�ýӿں��� Ϊ��ʹ�������Ӧ�ò�ӿڳ����׼������ܶ�����һЩ��׼���ĺ�����
	�����ֲ���µ�Ӳ����ʱӦ��driver.c��ʵ����Щ�������ܡ�
*/

/** \defgroup GenFrame ����¼������ӿ�

	�������д�������õĽӿں���
*/

/** \defgroup TimerTask ��ʱ�������
	��ʱ��������ʵ����ʱ�ֲ��������ܡ�
	����������Ҫʵ������һ�����ܣ���һ���¿�ϵͳ�У�����Ȱ���ǰ�ȴ򿪷������ʱ10���򿪵��ȣ�
	�ر�ʱ�ȹرյ���Ȱ�������ʱ30��ط��������Ҫ��رպ���3�����ڲ����ٴδ򿪡�
	ʹ�ö�ʱ����ʵ�ִ˹��ܵĴ������£�
	\code
	#include "CAFrame.h"

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
		TT_BEGIN(pTask); 

		drv_output(IO_Y1,1);//�򿪷��

		TT_DELAY(pTask,10);//��ʱ10��
		
		drv_output(IO_Y2,1);//�򿪵���
		g_RunStatus=1;//�������״̬

		TT_END(pTask);
	}
	
	//�ػ���������
	void CloseTask(PTIMER_TASK pTask)
	{
		TT_BEGIN(pTask);

		drv_output(IO_Y2,0);//�رյ���
		
		TT_DELAY(pTask,30); //��ʱ30��
		
		drv_output(IO_Y1,0);//�رշ��
		
		TT_DELAY(pTask,180);//�ȴ�3���ӱ���Ƶ������

		g_RunStatus=0;//�ػ����״̬
		
		TT_END(pTask);
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
	
	void InitApp(void)
	{
		//��ͨ��ʱ�¼����˶�ʱ����˹����޹أ������ڵ���
		frm_install_timer_event(0,2000,Timer1Event);
		
		//���ƶ�ʱ��
		frm_install_timer_event(1,1000,TaskTimerEvent);
		frm_bind_timer_task(1,&g_OpenTask,OpenTask); //���������
		frm_bind_timer_task(1,&g_CloseTask,CloseTask);//��ػ�����
		
		//��װ���ư��������¼�
		//IO_X1�����½��ش����¼�,�͵�ƽ���ٱ���5ms
		frm_install_input_event(IO_X1,InputEvent1,5,ET_FALLING_EDGE); 
	}

 	\endcode	
*/


/////////////////////////////////////////////////////////
//���(frame.c)�ӿں���//////////////////////////////////
/*
	�����¼��������ݽṹ(����ܳ���ʹ��)
*/
typedef struct tag_input_event
{
	//�˿ں�
	uint8_t port; 	
	//�������� 0:�½��ش�����1�������ش���  2�����ش���  3��������(����ѭ��)	
	uint8_t type;		
	//��ǰ״̬(0��1)
	uint8_t state;	
	//����ʱ��(ms)
	uint16_t hold;	
	//�˲�����(ms)
	uint8_t filter_config;	
	//�˲�������
	uint8_t filter_counter;	
	//�¼�������	
	void (*Func)(uint16_t);	
}EVENT_INPUT,*PEVENT_INPUT;
//�����¼��������ͷ��Ŷ���
#define ET_FALLING_EDGE	0
#define ET_RISING_EDGE	1
#define ET_ALL_EDGE			2
#define ET_DISABLE			3

/*
	��ʱ����������ݽṹ
*/
typedef struct tag_timer_task
{
	uint8_t busy;		//��������	
	uint16_t step; 	//��ǰִ�в���
	uint16_t counter; //��ʱ�ü�����
	
	void (*Func)(struct tag_timer_task *);//��������
	
	struct tag_timer_task *next; //��һ���ṹ
}TIMER_TASK,*PTIMER_TASK;

/*
	��ʱ���������ݽṹ(���ڿ�ܳ���ʹ��)
*/
typedef struct tag_timer_event
{
	//��ʱ�����
	uint8_t id;			
	//��ʱ�趨ֵ
	uint16_t cfg;		
	//��ʱ������
	uint16_t cnt;		
	//�¼�������
	void (*Func)(void);
	
	//��ʱ�����б�
	struct tag_timer_task *pTaskList;	
}EVENT_TIMER,*PEVENT_TIMER;

/*
	ͨ���¼��������ݽṹ
*/
typedef struct tag_event
{
	//�¼��������
	uint8_t flag;
	//�¼�������
	void (*Func)(void);
}EVENT,*PEVENT;

/** \ingroup InputDriver
	\brief ��ȡ��������˿�״̬�����ڶ�ȡ�˲���Ŀ�������״̬
	\param port �˿�:IO_X1 ~ IO_X6
	\return ���źŷ���1  ���źŷ���0
*/
uint8_t frm_input(uint8_t port);

/** \ingroup InputDriver
	\brief ��װ���������¼�
	\param port �˿ں�:IO_X1 ~ IO_X6
	\param fn �¼���������ԭ��: void func(uint16_t t)<br>����tΪ֮ǰ��ƽ״̬���ֵ�ʱ��(ms)���������ʱ�����0xFFFF��t����0xFFFF;
	\param filter �˲����������Ժ���Ϊ��λ���ź�ȷ��ʱ��
	\param type ET_FALLING_EDGE���½��ش���<BR>  ET_RISING_EDGE�������ش��� <BR>ET_ALL_EDGE�����ش���<BR>ET_DISABLE���¼�������ֹ(���Ӧ��ʱʹ��)
*/
void frm_install_input_event(uint8_t port,void (*fn)(uint16_t),uint8_t filter,uint8_t type);

/** \ingroup TimerDriver
	\brief ��װ��ʱ������
	\param id ��ʱ����� 0��ʼ�����ı��
	\param tms ��ʱ���¼���������(����Ϊ��λ)
	\param fn ��ʱ�¼�������<br>ԭ��:void func(void)
*/
void frm_install_timer_event(uint8_t id,uint16_t tms,void (*fn)(void));

/** \ingroup TimerDriver
	\brief ����ָ����ʱ��
	\param id ��ʱ�����
*/
void frm_timer_enable(uint8_t id);

/** \ingroup TimerDriver
	\brief ��ָֹ����ʱ��
	\param id ��ʱ�����
*/
void frm_timer_disable(uint8_t id);

/** \ingroup TimerDriver
	\brief ��ʱ�����¿�ʼ����
	\param id ��ʱ�����
*/
void frm_timer_reset(uint8_t id);

/** \ingroup GenFrame
	\brief ��װͨ���¼�
	\param id �¼�ID
	\param fn �¼�������<br>ԭ�ͣ�void func(void)
*/
void frm_install_event(uint8_t id,void (*fn)(void));

/** \ingroup GenFrame
	\brief ���1ms�δ��������˺���������������ÿһ�������һ��
	\note ����֧�ֿ�ܵ�������Ͷ�ʱ��������
*/
void frm_inc_timer_tick(void);

/** \ingroup GenFrame 
	\brief ����ͨ���¼����˺���ͨ�����������ж��е��ã�����ʵ�ֶ�Ӧ�¼��Ĵ���
	\param id �����¼�ID
*/
void frm_set_event(uint8_t id);

/** \ingroup GenFrame
	\brief ��ѯָ���¼��Ĵ���״̬
	\param id ��ѯ�¼�ID
*/
uint8_t frm_get_event_state(uint8_t id);


/////////////////////////////////////////////////////////
//����(driver.c)����ʵ�ֵĽӿں���///////////////////////
/** \ingroup GenDriver
	\brief ��ܳ�ʼ��ǰ���ã����ڳ��绯Ӳ���豸��������(driver.c)�б���ʵ�ִ˺�����
*/
void drv_init(void);

/** \ingroup GenDriver
	\brief ���Ź���λ���ڿ����ѭ���е��ã�������(driver.c)�б���ʵ�ִ˺�����
*/
void drv_wdt_reset(void);

/** \ingroup GenDriver
	\brief ��ȫ�ر��жϣ���ִ��һ�β�ϣ�����жϵĴ���ʱʹ�ã��˺�������drv_exit_critical���ʹ�á�������(driver.c)�б���ʵ�ִ˺�����
*/
void drv_enter_critical(void);

/** \ingroup GenDriver
	\brief �˳��жϱ��������,�˺�������drv_enter_critical���ʹ�á�������(driver.c)�б���ʵ�ִ˺�����
*/
void drv_exit_critical(void);

/** \ingroup InputDriver
	\brief ��ȡ��������˿�״̬��ֱ�Ӷ�ȡ�˿ڵ�ǰ�������źţ�������(driver.c)�б���ʵ�ִ˺�����
	\param port �˿�:IO_X1 ~ IO_X6
	\return ���źŷ���1  ���źŷ���0
*/
uint8_t drv_input(uint8_t port);

/** \ingroup OutputDriver
	\brief ������������ƺ�����������(driver.c)�б���ʵ�ִ˺�����
	\param port ����˿ں�(IO_Y1 ~ IO_Y6)
	\param val  ���ֵ  1������̵�������  0������̵����Ͽ�
*/
void drv_output(uint8_t port,uint8_t val);

/** \ingroup GenDriver
	\brief ��ȫ��ȡEEPROM�ӿں���,������(driver.c)�б���ʵ�ִ˺�����
	\param addr ��ȡ��ַ
	\param buf ��ȡ������
	\param size ��ȡ����(�ֽ�)
*/
void drv_eeprom_read(uint16_t addr,uint8_t *buf,uint8_t size);

/** \ingroup GenDriver
	\brief ��ȫд��EEPROM�ӿں���,������(driver.c)�б���ʵ�ִ˺�����
	\param addr д���ַ
	\param buf д�뻺����
	\param size д�볤��(�ֽ�)
*/
void drv_eeprom_write(uint16_t addr,uint8_t *buf,uint8_t size);

/** \ingroup TimerTask
	\brief ��ʼִ�ж�ʱ����
	\param pTask ��ʱ��������ַ
*/
void frm_begin_timer_task(PTIMER_TASK pTask);

/** \ingroup TimerTask
	\brief ָֹͣ���Ķ�ʱ����
	\param pTask ��ʱ��������ַ
*/
void frm_end_timer_task(PTIMER_TASK pTask);

/** \ingroup TimerTask	
 * ���ö�ʱ����ĵ�ǰִ�в���
 * 
 * \note �˺���Ϊ�����ϰ������������������Ӧ����ʹ�ã���
 * \hideinitializer
*/
#define frm_timer_task_setstep(X,Y) (X->step=Y,X->counter=0)

/** \ingroup TimerTask	
 * ���ö�ʱ����ת������һ��ִ�в��衡
 * 
 * \note �˺���Ϊ�����ϰ������������������Ӧ����ʹ�ã�
 * \hideinitializer
*/
#define frm_timer_task_nextstep(X)  (X->step++,X->counter=0)

/** \ingroup TimerTask
	\brief ��ʱ����󶨵���ʱ��
	\param id ��ʱ�����
	\param pTask ��ʱ��������ַ
	\param fn ��ʱ��������
*/
void frm_bind_timer_task(uint8_t id,PTIMER_TASK pTask,void (*fn)(PTIMER_TASK));

/** \ingroup TimerTask	
 * ��ʱ����ʼ�����ڶ�ʱ���������Ŀ�ʼ�����ã�
 * \note �˺���Ϊ�����֧�ṹ��������������������ѭ���Ƚṹ�����ʹ�ã�
 * 
 * \param T ��ʱ��������ַ
 * \hideinitializer
*/
#define TT_BEGIN(T) switch(T->step){case 0:

/** \ingroup TimerTask
 * ��ʱ������������ڶ�ʱ����Ľ��������ã��ô˺�����Ķ�ʱ����˳��ִ����ɺ��˳���
 * \note �˺���Ϊ�����֧�ṹ��������������������ѭ���Ƚṹ�����ʹ�ã�
 * 
 * \param T ��ʱ��������ַ
 * \hideinitializer
*/	
#define TT_END(T) default:frm_end_timer_task(T);break;}

/** \ingroup TimerTask	
 * ��ʱ����ѭ������,�ڶ�ʱ����Ľ��������ã��ô˺�����Ķ�ʱ����˳��ִ����ɺ��Զ����»ص���ʼ������ִ�У�
 * �����Զ�������
 * \note �˺���Ϊ�����֧�ṹ��������������������ѭ���Ƚṹ�����ʹ�ã�
 * 
 * \param T ��ʱ��������ַ
 * \hideinitializer
*/
#define TT_LOOP(T) default:T->step=0;T->counter=0;break;}

/** \ingroup TimerTask	
 * ��ʱ�����еȴ�ĳ������
 * \note �˺���Ϊ�����֧�ṹ��������������������ѭ���Ƚṹ�����ʹ�ã�
 * 
 * \param T ��ʱ��������ַ
 * \param Y �������ʽ����Y�Ľ��Ϊ0ʱ�ȴ�������0ʱ�˳��ȴ���
 * \hideinitializer
*/
#define TT_WAIT(T,Y) T->step=__LINE__; case __LINE__:\
										if(!(Y)) return;\
										T->counter=0

/** \ingroup TimerTask	
 * ��ʱ��������ʱ����
 * ע���˺���Ϊ�����֧�ṹ��������������������ѭ���Ƚṹ�����ʹ�ã�
 * 
 * \param T ��ʱ��������ַ
 * \param Y ��ʱ���ڣ���������ʵ��Y�ζ�ʱ����������󶨵Ķ�ʱ����������ʱ��
 * \hideinitializer
*/										
#define TT_DELAY(T,Y) T->step=__LINE__; case __LINE__:\
										if(T->counter < (Y)) return ;\
										T->counter=0 

/** \ingroup TimerTask	
 * ��ʱ�������¿�ʼִ�У����´�TT_BEGIN����ʼִ�У�
 * 
 * \param T ��ʱ��������ַ
 * \hideinitializer
*/										
#define TT_RESTART(T) T->step=0,T->counter=0;return 
	
/** \ingroup TimerTask	
 * ��ʱ�����˳�����
 * 
 * \param T ��ʱ��������ַ
 * \hideinitializer
*/				
#define TT_EXIT(T) frm_end_timer_task(T);return 

/** \ingroup GenFrame
	\brief ��ʼ����ܣ��й�Ӧ�����������
	\note ��Ҫ��main������ʼ������
*/
void CAFrameInit(void);

/** \ingroup GenFrame
	\brief ��ܹ�����ѭ�����й�Ӧ�����������
	\note main������ѭ���е���
*/
void CAFramePoll(void);	
		
#endif
