/* 
 * Copyright (c) 2006-2018, RT-Thread Development Team 
 * 
 * SPDX-License-Identifier: Apache-2.0 
 * 
 * Change Logs: 
 * Date           Author        Notes 
 * 2018-08-24     yangjie       the  first   version
 * 2025-03-10     liutentteng   the  second  version 
 */ 
 
/*
 * 程序清单：动态内存管理例程
 *
 * 这个程序会创建一个动态的线程，这个线程会动态申请内存并释放
 * 每次申请更大的内存，当申请不到的时候就结束
 */
#include <rtthread.h>

#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

/* 线程入口 */
void thread1_entry(void *parameter)
{
    int i;
	char *ptr;
	
    void *p = rt_malloc(sizeof(int)); // 分配一个 int 大小的内存
	if (p != NULL) {
		*(int *)p = 100; // 将 void * 转换为 int *，然后存储数据
		int data = *(int *)p; // 获取指针指向地址中的数据
		rt_kprintf("Data: %d\n", data); // 输出: Data: 100
		rt_free(p); // 释放内存
	}
	
	p = rt_calloc(3,sizeof(int)); // 分配一个 int 大小的内存 都存放0
	if (p != NULL) {
		int *p1 = (int*) p; // 要从void转成int才能加减
		int data = *p1; // 获取指针指向地址中的数据
		rt_kprintf("Data1: %d, Address: %p \n", data, p1); // 输出: Data1: 0 Address: 20005d38
		p1++;
		data = *p1;
		rt_kprintf("Data2: %d, Address: %p \n",data, p1);	//输出 Data2: 0, Address: 20005d3c
		rt_free(p); // 释放内存
	}
	
	p = rt_malloc(sizeof(int));
	if(p!=NULL){
		int *p1 = (int*)p;
		int data = *p1;
		rt_memset(p,16,sizeof(int));
		rt_kprintf("Data1: %x, Address: %p \n", data, p1); 
		p = rt_realloc(p, 2);
		data = *p1;
		rt_kprintf("Data2: %x, Address: %p \n",data, p1);
		rt_free(p);
	}
	
	
    for (i = 0; ; i++)
    {
        /* 每次分配 (1 << i) 大小字节数的内存空间 */
		/* (1 << i) 表示将数字1向左移动 i 位置*/
       ptr = rt_malloc(1 << i);
		/*采用右移动策略*/
		//ptr = rt_malloc(0x80000000>>(31-i));

        /* 如果分配成功 */
        if (ptr != RT_NULL)
        {
            rt_kprintf("get memory :%d byte\n", (1 << i));
			//rt_kprintf("get memory :%d byte\n", (0x80000000>>(31-i)));
			rt_memset(ptr,0,(1<<i));
            /* 释放内存块 */
            rt_free(ptr);
            rt_kprintf("free memory :%d byte\n", (1 << i));
			//rt_kprintf("free memory :%d byte\n", (0x80000000>>(31-i)));
            ptr = RT_NULL;
        }
        else
        {
            rt_kprintf("try to get %d byte memory failed!\n", (1 << i));
			//rt_kprintf("try to get %d byte memory failed!\n", (0x80000000>>(31-i)));
            return;
        }
    }
}

int dynmem_sample(void)
{
    rt_thread_t tid;

    /* 创建线程1 */
    tid = rt_thread_create("thread1",
                           thread1_entry, RT_NULL,
                           THREAD_STACK_SIZE,
                           THREAD_PRIORITY,
                           THREAD_TIMESLICE);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
/* 导出到 msh 命令列表中 */
/* 用户只需要在命令行中输入dynmen_sample即可调用示例函数 */
MSH_CMD_EXPORT(dynmem_sample, dynmem sample);
