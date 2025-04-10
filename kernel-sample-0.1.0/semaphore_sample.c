/* 
 * Copyright (c) 2006-2018, RT-Thread Development Team 
 * 
 * SPDX-License-Identifier: Apache-2.0 
 * 
 * Change Logs: 
 * Date           Author       Notes 
 * 2018-08-24     yangjie      the first version 
 * 2025-04-09	  RINZEORN     change
 */ 

/*
 * 程序清单：信号量例程
 *
 * 该例程创建了一个动态信号量，初始化两个线程，线程1在count每计数10次时，
 * 发送一个信号量，线程2在接收信号量后，对number进行加1操作
 * 增加一个线程3和一个动态信号量test_sem，来测试线程递归调用所出现的死锁问题
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_TIMESLICE        1	//5

/* 指向信号量的指针 */
static rt_sem_t dynamic_sem = RT_NULL;
static rt_sem_t test_sem = RT_NULL;

//static rt_thread_t thread4 = RT_NULL;

ALIGN(RT_ALIGN_SIZE)	//对齐
static char thread1_stack[1024];
static struct rt_thread thread1;	//线程1静态
static void rt_thread1_entry(void *parameter)
{
    static rt_uint8_t count = 0;
  
    while(1)
    {
        if(count <= 100)
        {
            count++;           
        }
        else
            return; 
        
        /* count每计数10次，就释放一次信号量 */
         if(0 == (count % 10))
        {
            rt_kprintf("t1 release a dynamic semaphore.\n" ); 
            rt_sem_release(dynamic_sem);            
        }
    }
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;
static void rt_thread2_entry(void *parameter)
{
    static rt_err_t result;
    static rt_uint8_t number = 0;
    while(1)
    {
        /* 永久方式等待信号量，获取到信号量，则执行number自加的操作 */
        result = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER);
        /* 必须判断信号量是否是 RT_EOK 才形成了保护的效果 */
		if (result != RT_EOK)
        {        
            rt_kprintf("t2 take a dynamic semaphore, failed.\n");
            rt_sem_delete(dynamic_sem);
            return;
        }
        else
        {      
            number++;             
            rt_kprintf("t2 take a dynamic semaphore. number = %d\n" ,number);                        
        }
    }   
}


/* 线程3用于测试递归调用信号量 死锁 */
ALIGN(RT_ALIGN_SIZE)
static char thread3_stack[1024];
static struct rt_thread thread3;
static void rt_thread3_entry(void *parameter)
{
	static rt_err_t flag;
    while(1)
    {
        /* 信号量test_sem初始为1 */
		/* 永久方式等待信号量，获取到信号量 */
		flag = rt_sem_take(test_sem, RT_WAITING_FOREVER);
		    /* test_sem.value = 0 */
            /* 无法获取到信号量 自动锁死 */ 
            /* 代码循环 */
		if(flag != RT_EOK)
		{	
			rt_kprintf("faild, flag is: %ld\n",flag);
		}
		else
		{
			rt_kprintf("success, flag is: %ld\n",flag);
		}
		
		flag = rt_sem_take(test_sem, RT_WAITING_NO);
				
		if(flag != RT_EOK)
		{	
			rt_kprintf("faild, flag is: %ld\n",flag);
		}
		else
		{
			rt_kprintf("success, flag is: %ld\n",flag);
		}
		
//		flag =  rt_sem_take(test_sem, RT_WAITING_NO);
		
		/*会挂起*/
//		flag = rt_sem_take(test_sem, 5);
		/*如果等待设置为永久 则会自动挂起该线程3 反而能执行其他的线程*/
		flag = rt_sem_take(test_sem, RT_WAITING_FOREVER);
		if(flag != RT_EOK)
		{	
			rt_kprintf("faild, flag is: %ld\n",flag);
		}
		else
		{
			rt_kprintf("success, flag is: %ld\n",flag);
		}
	
		
				/* 有释放也不会挂起 */
				/* 可以试试注释下方代码后的效果 */
//		        rt_sem_release(test_sem);
				rt_kprintf("4\n");
			
//			rt_sem_release(test_sem);
			rt_kprintf("5\n");
		
//		rt_sem_release(test_sem);
		rt_kprintf("6\n");
   }
}


/* 信号量示例的初始化 */
int semaphore_sample()
{
    /* 创建一个动态信号量，初始值是0 */
    dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO);
    
	if (dynamic_sem == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. dynamic semaphore value = 0.\n");
    }
	
	
	test_sem = rt_sem_create("sem", 1, RT_IPC_FLAG_FIFO);
	if (test_sem == RT_NULL)
    {
        rt_kprintf("create test_sem semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. test_sem semaphore value = 0.\n");
    }
	
	
	
    rt_thread_init(&thread1,
                   "thread1",
                   rt_thread1_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack), 
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);
                   
    rt_thread_init(&thread2,
                   "thread2",
                   rt_thread2_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack), 
                   THREAD_PRIORITY-1, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);

				   
    rt_thread_init(&thread3,
                   "thread3",
                   rt_thread3_entry,
                   RT_NULL,
                   &thread3_stack[0],
                   sizeof(thread2_stack), 
                   THREAD_PRIORITY-2, THREAD_TIMESLICE);
    rt_thread_startup(&thread3);
				   
				   
	return 0;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(semaphore_sample, semaphore sample);



int bool_semaphore_sample()
{
    /* 创建一个动态信号量，初始值是1 */
	test_sem = rt_sem_create("sem", 1, RT_IPC_FLAG_FIFO);
	if (test_sem == RT_NULL)
    {
        rt_kprintf("create test_sem semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. test_sem semaphore value = 0.\n");
    }
	rt_thread_init(&thread3,
                   "thread3",
                   rt_thread3_entry,
                   RT_NULL,
                   &thread3_stack[0],
                   sizeof(thread3_stack), 
                   THREAD_PRIORITY-2, THREAD_TIMESLICE);
    rt_thread_startup(&thread3);
	
//	thread4 = rt_thread_create("thread4", 
//				   rt_thread3_entry, 
//				   RT_NULL,
//				   sizeof(thread3_stack),
//				THREAD_PRIORITY-2, THREAD_TIMESLICE);
//	if(thread4 != RT_NULL)
//	{
//		rt_thread_startup(thread4);
//	}
				   
	return 0;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(bool_semaphore_sample, semaphore sample);
