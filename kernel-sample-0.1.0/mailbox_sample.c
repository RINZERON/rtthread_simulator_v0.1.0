/* 
 * Copyright (c) 2006-2018, RT-Thread Development Team 
 * 
 * SPDX-License-Identifier: Apache-2.0 
 * 
 * Change Logs: 
 * Date           Author       Notes 
 * 2018-08-24     yangjie      the first  version 
 * 2025-04-11     RINZERON     the second version
 */ 

/*
 * 程序清单：邮箱例程
 *
 * 这个程序会创建2个动态线程，一个静态的邮箱对象，其中一个线程往邮箱中发送邮件，
 * 一个线程往邮箱中收取邮件。
 */
#include <rtthread.h>

#define THREAD_PRIORITY      10
#define THREAD_TIMESLICE     5

/* 邮箱控制块 */
static struct rt_mailbox mb;
/* 用于放邮件的内存池 */
static char mb_pool[32];	// 32个字节

static char mb_str1[] = "I'm a mail!";
static char mb_str2[] = "this is another mail!";
static char mb_str3[] = "over";

static char mb_str_emergency[] = "emergency";

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;

/* 线程1入口 */
static void thread1_entry(void *parameter)
{
    char *str;

    while (1)
    {
        rt_kprintf("thread1: try to recv a mail\n");

        /* 从邮箱中收取邮件 */
        if (rt_mb_recv(&mb, (rt_uint32_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("thread1: get a mail from mailbox, the content:%s\n", str);
            if (str == mb_str3)
                break;

            /* 延时100ms */
            rt_thread_mdelay(100);
        }
    }
    /* 执行邮箱对象脱离 */
    rt_mb_detach(&mb);
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;

/* 线程2入口 */
static void thread2_entry(void *parameter)
{
    rt_uint8_t count;
	rt_err_t result;
	
    count = 0;
    while (count < 8)
    {
        count ++;
        if (count & 0x1)
        {
            /* 发送mb_str1地址到邮箱中 */
            rt_mb_send(&mb, (rt_uint32_t)&mb_str1);
        }
        else
        {
            /* 发送mb_str2地址到邮箱中 */
            rt_mb_send(&mb, (rt_uint32_t)&mb_str2);
        }
		
		if (count == 8 )
		{
			while(1)
			{
				result = rt_mb_send_wait(&mb, (rt_uint32_t)&mb_str_emergency,10);
				if(result == RT_EOK)
				{
					rt_kprintf("%x\n",result);
				}
				else
				{
					rt_kprintf("%x\n",result);
					count++;
					break;
				}
			}
		}
		
        /* 延时200ms */
        rt_thread_mdelay(200);
    }

    /* 发送邮件告诉线程1，线程2已经运行结束 */
    rt_mb_send(&mb, (rt_uint32_t)&mb_str3);
}

int mailbox_sample(void)
{
    rt_err_t result;

    /* 初始化一个mailbox */
    result = rt_mb_init(&mb,
                        "mbt",                      /* 名称是mbt */
                        &mb_pool[0],                /* 邮箱用到的内存池是mb_pool */
                        sizeof(mb_pool) / 4,        /* 邮箱中的邮件数目，因为一封邮件占4字节 */
						RT_IPC_FLAG_PRIO			 /* 采用PRIO方式进行线程等待 */                        
//						RT_IPC_FLAG_FIFO			 /* 采用FIFO方式进行线程等待 */
						);         
    if (result != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }

    rt_thread_init(&thread1,
                   "thread1",
                   thread1_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack),
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);

    rt_thread_init(&thread2,
                   "thread2",
                   thread2_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack),
                   THREAD_PRIORITY-1, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(mailbox_sample, mailbox sample);
