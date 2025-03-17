
## 详细解释`rt_thread_yield()`和`rt_schedule()`的实现和作用

一个原则：**就绪**的、**优先级最高**的线程**先**执行。<p>
`rt_thread_yield()`函数的示例代码在`threadyield_sample.c`文件中。


### `rt_thread_yield()`在`thread.c`文件中，使线程让出处理器资源
```C
/**
 * This function will let current thread yield processor, and scheduler will 
 * choose a highest thread to run. After yield processor, the current thread
 * is still in READY state.
 *
 * @return RT_EOK
 */
/*这个函数会让当前线程让出处理器，并让调度器选择“最高”（最靠前）的线程来运行*/
/*让出处理器资源后，当前线程会处于就绪状态*/
rt_err_t rt_thread_yield(void)
{
    register rt_base_t level;   //用于中断
    struct rt_thread *thread;   //指向线程控制块的指针

    /* disable interrupt */
    /* 禁用中断 */
    level = rt_hw_interrupt_disable();

    /* set to current thread */
    /* 设定为“当前”的线程 */
    /* rt_current_thread指向的线程*/
    thread = rt_current_thread;

    /* if the thread stat is READY and on ready queue list */
    /* 如果线程状态为就绪态 并且处于就绪的线程队列表中 */
    /* 线程状态判断依靠按位与操作 state & 0x0f(mask) == 0x01(ready) */
    /* 线程列表不只有当前线程 */
    if ((thread->stat & RT_THREAD_STAT_MASK) == RT_THREAD_READY &&
        thread->tlist.next != thread->tlist.prev)
    {
        /* remove thread from thread list */
        /* 从线程列表中移除线程 */
        rt_list_remove(&(thread->tlist));

        /* put thread to end of ready queue */
        /* 将线程放入当前“优先级就绪态队列表”的最后一位 */
        rt_list_insert_before(&(rt_thread_priority_table[thread->current_priority]),
                              &(thread->tlist));

        /* enable interrupt */
        /* 使能中断 */
        rt_hw_interrupt_enable(level);

        /* 调用rt_schedule()来完成线程调度 */
        rt_schedule();

        return RT_EOK;
    }

    /* 无事发生 xs 就绪队列为空 或者*/
    /* enable interrupt */
    /* 使能中断 */
    rt_hw_interrupt_enable(level);
    /* 继续当前线程运行 */

    return RT_EOK;
}
RTM_EXPORT(rt_thread_yield);
```

> 注意：在 RT-Thread(v4.0.0) 版本以下没有运行状态
> simulator 我们使用的模拟器 RT-Thread 内核版本是 3.1.0




### `rt_schedule()`在`scheduler.c`文件中
```C
/**
 * This function will perform one schedule. It will select one thread
 * with the highest priority level, then switch to it.
 */
/*
    这个函数会实现一个调度器，这个调度器会选择最高优先级的线程，并在之后转换到这个线程上。
*/
void rt_schedule(void)
{
    rt_base_t level;                //中断相关
    struct rt_thread *to_thread;    //下一个要转换去的线程
    struct rt_thread *from_thread;  //上一个从转换来的线程

    /* disable interrupt */
    /* 禁用中断 */
    level = rt_hw_interrupt_disable();  

    /* check the scheduler is enabled or not */
    /* 检查调度器是否可用 */
    if (rt_scheduler_lock_nest == 0)
    {
        register rt_ubase_t highest_ready_priority;

/*
typedef unsigned long   rt_uint32_t; //< 32bit unsigned integer type 
rt_uint32_t rt_thread_ready_priority_group;
rt_uint8_t rt_thread_ready_table[32];   // 大于32个优先级才设 从而实现最高256个优先级
优先级是通过一个32位无符号数按位操作辨识的
*/
#if RT_THREAD_PRIORITY_MAX <= 32
        highest_ready_priority = __rt_ffs(rt_thread_ready_priority_group) - 1;
#else
        register rt_ubase_t number;

        number = __rt_ffs(rt_thread_ready_priority_group) - 1;
        highest_ready_priority = (number << 3) + __rt_ffs(rt_thread_ready_table[number]) - 1;
#endif

        /* get switch to thread */
        /* 得到要转换去的线程 */
        to_thread = rt_list_entry(rt_thread_priority_table[highest_ready_priority].next,
                                  struct rt_thread,
                                  tlist);

        /* if the destination thread is not the same as current thread */
        /* 如果目标线程不等于当前线程 */
        if (to_thread != rt_current_thread)
        {
            /* switch to new thread */
            /* 转换去新的线程 */
            rt_current_priority = (rt_uint8_t)highest_ready_priority;
            from_thread         = rt_current_thread;
            rt_current_thread   = to_thread;

            /* 调用系统调度钩子函数 如果有设置效果的话*/
            RT_OBJECT_HOOK_CALL(rt_scheduler_hook, (from_thread, to_thread));

            /* DEBUG LOG 记录日志 */
            /* to_thread->name 线程名称  to_thread->sp 线程栈指针*/
            /* rt_interrupt_nest 中断嵌套层数 */
            RT_DEBUG_LOG(RT_DEBUG_SCHEDULER,
                         ("[%d]switch to priority#%d "
                          "thread:%.*s(sp:0x%p), "
                          "from thread:%.*s(sp: 0x%p)\n",
                          rt_interrupt_nest, highest_ready_priority,
                          RT_NAME_MAX, to_thread->name, to_thread->sp,
                          RT_NAME_MAX, from_thread->name, from_thread->sp));

#ifdef RT_USING_OVERFLOW_CHECK
            /* 检查线程栈有没有溢出 */
            _rt_scheduler_stack_check(to_thread);
#endif

            if (rt_interrupt_nest == 0)
            {
                extern void rt_thread_handle_sig(rt_bool_t clean_state);

                /* 上下文切换 */
                rt_hw_context_switch((rt_uint32_t)&from_thread->sp,
                                     (rt_uint32_t)&to_thread->sp);

                /* 使能中断 */
                rt_hw_interrupt_enable(level);

#ifdef RT_USING_SIGNALS
                /* check signal status */
                /* 检查信号状态 */
                rt_thread_handle_sig(RT_TRUE);
#endif
            }
            else
            {
                RT_DEBUG_LOG(RT_DEBUG_SCHEDULER, ("switch in interrupt\n"));

                /* 中断的上下文切换 */
                rt_hw_context_switch_interrupt((rt_uint32_t)&from_thread->sp,
                                               (rt_uint32_t)&to_thread->sp);
                /* 使能中断 */
                rt_hw_interrupt_enable(level);
            }
        }
        else
        {
            /* 使能中断 */
            rt_hw_interrupt_enable(level);
        }
    }
    else
    {
        /* enable interrupt */
        /* 使能中断 */
        rt_hw_interrupt_enable(level);
    }
}
```

上下文切换函数解释参考：<p>
[RT-Thread上下文切换详解](https://blog.csdn.net/kouxi1/article/details/123742937)<p>
[线程定义和线程切换](https://doc.embedfire.com/rtos/rtthread/zh/latest/zero_to_one/switching_thread.html)<p>
