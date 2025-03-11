/* 
 * Copyright (c) 2006-2018, RT-Thread Development Team 
 * 
 * SPDX-License-Identifier: Apache-2.0 
 * 
 * Change Logs: 
 * Date           Author       Notes 
 * 2018-08-24     yangjie      the first version 
 */ 

/*
 * 程序清单：创建/删除、初始化线程
 *
 * 这个例子会创建两个线程，一个动态线程，一个静态线程。
 * 一个线程在运行完毕后自动被系统删除，另一个线程一直打印计数。
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;

/* 线程1的入口函数 */
static void thread1_entry(void *parameter)
{
    rt_uint32_t count = 0;

    while (1)
    {
        /* 线程1采用低优先级运行，一直打印计数值 */
        rt_kprintf("thread1 count: %d\n", count ++);
        rt_thread_mdelay(500); //延时500ms
    }
}

/* 线程1的入口函数 */
static void thread3_entry(void *parameter)
{
    rt_uint32_t count = 0;

    while (count<100)
    {
        /* 线程3采用最高优先级运行，打印计数值到100 */
        rt_kprintf("thread3 count: %d\n", count+=10);
        rt_thread_mdelay(500); //延时500ms
		/*线程3让出*/
    }
}


ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];	//分配线程栈的空间
static struct rt_thread thread2;	//静态定义线程控制块
/* 线程2入口 */
static void thread2_entry(void *param)
{
    rt_uint32_t count = 0;

    /* 线程2拥有较高的优先级，以抢占线程1而获得执行 */
    for (count = 0; count < 10 ; count++)
    {
        /* 线程2打印计数值 */
        rt_kprintf("thread2 count: %d\n", count);
    }
	/*线程3抢占第一 让出 然后线程2抢占*/
	/*不设置delay 一直运行结束才能让线程1运行*/
	/*线程1让出后 线程3运行 线程3让出 线程1运行 ---*/

    rt_kprintf("thread2 exit\n");
    /* 线程2运行结束后也将自动被系统删除
    (线程控制块和线程栈依然在idle线程中释放) */
}

/* 删除线程示例的初始化 */
int thread_sample(void)
{
    /* 创建动态线程1，名称是thread1，入口是thread1_entry*/
    tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    
    /* 如果获得线程控制块，启动这个线程 */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    /* 初始化静态线程2，名称是thread2，入口是thread2_entry */
    rt_thread_init(&thread2,
                   "thread2",
                   thread2_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack),
                   THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);
				   
	/*创建动态线程3*/
	tid3 = rt_thread_create("thread3",
             thread3_entry, RT_NULL,
             THREAD_STACK_SIZE,
             THREAD_PRIORITY - 2, THREAD_TIMESLICE);
	 /* 如果获得线程控制块，启动这个线程 */
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_sample, thread sample);
