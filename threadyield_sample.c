
/*
 * �����嵥�������̹߳�����
 *
 * ������ӻᴴ�������̣߳�������̬�߳�
 * һ���̻߳�ִ��һ�κ������һ���߳�һֱ��ӡ������
 */


#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid_test = RT_NULL;

/* �߳�1����ں��� */
static void thread1_entry(void *parameter)
{
    rt_uint32_t count = 0;
	
    while (1)
    {
		tid_test = rt_thread_self();
		rt_kprintf("Running thread is: %s state is :%x \n", tid_test->name, tid_test->stat);
        /* �߳�1���õ����ȼ����У�һֱ��ӡ����ֵ */
        rt_kprintf(" count: %d\n", count ++);
        if(count %5 ==0)
		{
			//rt_kprintf("Yield thread is: %s state is :%x \n", rt_thread_self()->name, rt_thread_self()->stat);
			//rt_thread_yield();
			rt_thread_suspend(tid1);
			rt_kprintf("suspend thread is: %s state is :%x \n", tid1->name, tid1->stat);
			rt_schedule();
		}
    }
}

/* �߳�2����ں��� */
static void thread2_entry(void *parameter)
{
	
	while (1)
    {	
		/* thread1 ״̬stat is ready 0x01*/
		if(tid_test!=RT_NULL)
		{
			rt_kprintf("test thread is: %s, state is: %x\n",tid_test->name,tid_test->stat);
		}
		if(tid1!=RT_NULL)
		{
			rt_kprintf("tid1 thread is: %s, state is: %x\n",tid1->name,tid1->stat);
		}
		
		/*rt_thread_self()��ȡ��ǰ���е��߳�*/
		rt_kprintf("Running thread is: %s, state is: %x\n",rt_thread_self()->name,rt_thread_self()->stat); 
		rt_thread_resume(tid1);
		rt_thread_yield();
		//rt_thread_mdelay(50);
    }
}

/* �̴߳��� */
int threadyield_sample(void)
{
    /* ������̬�߳�1��������thread1�������thread1_entry*/
    tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    
    /* �������߳̿��ƿ飬��������߳� */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

				   
	/*������̬�߳�2*/
	tid2 = rt_thread_create("thread2",
             thread2_entry, RT_NULL,
             THREAD_STACK_SIZE,
             THREAD_PRIORITY, THREAD_TIMESLICE);
	 /* �������߳̿��ƿ飬��������߳� */
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);
	
	return 0;
}

/* ������ msh �����б��� */
MSH_CMD_EXPORT(threadyield_sample, thread yield sample);
