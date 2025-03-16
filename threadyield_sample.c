
/*
 * 程序清单：测试线程挂起函数
 *
 * 这个例子会创建两个线程，两个动态线程
 * 一个线程会执行一段后挂起，另一个线程一直打印计数。
 */


#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid_test = RT_NULL;

/* 线程1的入口函数 */
static void thread1_entry(void *parameter)
{
    rt_uint32_t count = 0;
	
    while (1)
    {
		tid_test = rt_thread_self();
		rt_kprintf("Running thread is: %s state is :%x \n", tid_test->name, tid_test->stat);
        /* 线程1采用低优先级运行，一直打印计数值 */
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

/* 线程2的入口函数 */
static void thread2_entry(void *parameter)
{
	
	while (1)
    {	
		/* thread1 状态stat is ready 0x01*/
		if(tid_test!=RT_NULL)
		{
			rt_kprintf("test thread is: %s, state is: %x\n",tid_test->name,tid_test->stat);
		}
		if(tid1!=RT_NULL)
		{
			rt_kprintf("tid1 thread is: %s, state is: %x\n",tid1->name,tid1->stat);
		}
		
		/*rt_thread_self()获取当前运行的线程*/
		rt_kprintf("Running thread is: %s, state is: %x\n",rt_thread_self()->name,rt_thread_self()->stat); 
		rt_thread_resume(tid1);
		rt_thread_yield();
		//rt_thread_mdelay(50);
    }
}

/* 线程创建 */
int threadyield_sample(void)
{
    /* 创建动态线程1，名称是thread1，入口是thread1_entry*/
    tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    
    /* 如果获得线程控制块，启动这个线程 */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

				   
	/*创建动态线程2*/
	tid2 = rt_thread_create("thread2",
             thread2_entry, RT_NULL,
             THREAD_STACK_SIZE,
             THREAD_PRIORITY, THREAD_TIMESLICE);
	 /* 如果获得线程控制块，启动这个线程 */
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);
	
	return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(threadyield_sample, thread yield sample);
