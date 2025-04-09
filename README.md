# 学习RT-Thread


学习笔记放入docs文件夹中

参考链接：

[RT-Thread文档中心](https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/README)

[RT-Thread开源项目](https://github.com/RT-Thread/rt-thread)


## 文件夹目录

applications|		用户应用代码
----|-----
drivers	|		RT-Thread的驱动，不同平台底层驱动具体实现|
kernel-sample	|	教程内核例程源码
Libraries		|	STM32的芯片固件库，HAL库
rt-thread		|	RT-Thread的源码
docs            |   学习笔记



## 工程目录
Kell 打开 project.uvprojx
左侧有工程目录

|Applications|  ./applications|
|----|----|
|Drivers|		./drivers|
STM32-HAL|	Libraries中的STM32库文件
Kernel|		./rt-thread/src
CORTEX-M3|	./rt-thread/libcpu
DeviceDrivers|	./rt-thread/components/drivers|
finsh|		./rt-thread/components/finsh
kernel-sample|	./kernel-sample-0.1.0


## 调试代码注意事项

1. 在Keil仿真调试过程中，无法直接修改代码。必须退出调试模式，修改代码后重新编译和下载程序，才能继续调试。这是由调试器的工作机制决定的，适用于大多数嵌入式开发环境。

## 什么是实时操作系统？/嵌入式系统的实时性体现在哪里？

实时操作系统

## 为什么要“对齐”？
在嵌入式操作系统中，我们经常能看到`32位对齐`、`64位对齐`、`8位对齐`等等这些术语。