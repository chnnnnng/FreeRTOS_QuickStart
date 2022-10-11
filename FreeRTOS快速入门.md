# FreeRTOS快速入门

[TOC]

## 实时操作系统

FreeRTOS作为实时操作系统，除了具备一般操作系统的基本功能——如处理器管理、存储器管理等，重点突出的是其**实时性**。

实时性体现在两个方面：

- 任务切换的实时性
- 中断响应的实时性

这两点实时性都有FreeRTOS独特的**任务调度策略**保障。

任务调度：操作系统按照系统节拍，根据一定的顺序，切换运行的上下文。

FreeRTOS的任务调度策略可以总结为：**固定优先级、抢占式调度、相同优先级时间片轮询**。

### 任务切换的实时性如何保证？

相比于Linux操作系统使用“完全公平的调度策略”——即完全的时间片轮询策略，FreeRTOS则加入了**抢占**的机制，即高优先级的任务可以优先得到运行。

![截屏2022-10-11 下午2.43.18](/Users/chng/Library/Application Support/typora-user-images/截屏2022-10-11 下午2.43.18.png)

（上图：Linux的时间片轮询机制）

FreeRTOS每次任务切换，会在当前的就绪列表中，选出优先级最高的一组任务，在这组任务中进行时间片轮询；只有当高优先级的任务全部阻塞或挂起（即暂时移出就绪列表），低优先级的任务才能进行时间片轮询。

![截屏2022-10-11 下午2.48.12](/Users/chng/Library/Application Support/typora-user-images/截屏2022-10-11 下午2.48.12.png)

（上图：FreeRTOS的调度机制）

开发者通过为需要实时处理的任务设置一个较高的优先级，来保证其实时性。

### 中断响应的实时性如何保证？

- **假设不存在外部中断，处理器的调度如下图：**

 ![截屏2022-10-10 上午11.31.37](/Users/chng/Library/Application Support/typora-user-images/截屏2022-10-10 上午11.31.37.png)

Systick中断由硬件产生，优先级最高，在Systick中断中切换上下文。

在没有外部中断的前提下，任务可以按照“固定优先级、抢占式调度、相同优先级时间片轮询”的方式，正常调度。

- **现在引入了外部中断**

![截屏2022-10-10 上午11.33.25](/Users/chng/Library/Application Support/typora-user-images/截屏2022-10-10 上午11.33.25.png)

在外部中断处理程序运行过程中，如果发生了systick中断并进行了上下文切换，原本的中断处理程序会被打断，实时性得不到保障。

同时，在IRQ中断活跃的时候企图回到线程状态，会触发Fault异常。

解决方法：是否可以降低systick的优先级？

- **有外部中断，但是降低了systick的优先级**

![截屏2022-10-10 上午11.37.58](/Users/chng/Library/Application Support/typora-user-images/截屏2022-10-10 上午11.37.58.png)

看似没有问题，但是，在systick中，系统进行任务调度是需要耗时的，而且在systick中，操作系统需要“**进入临界区**”，也就是**关闭所有中断**，这就导致了在这段时间里如果发生了IRQ，将不会被处理——也不够实时。

- **最佳实践：Systick+SVC+PenSV**（针对arm cortex m系列内核）

![截屏2022-10-10 上午11.41.58_副本](/Users/chng/Desktop/截屏2022-10-10 上午11.41.58_副本.jpg)

SVC：系统服务调用，用于调用一些在特权模式下的程序。

PendSV：可延期的系统服务。在SVC或Systick申请一个PendSV调用，PendSV不会立即执行，而是会等到没有其他优先级更高的中断的情况下才会执行。

系统从SVC开始，申请一个PendSV，在没有其他中断的时候，PendSV执行，进行上下文切换；随后在Systick中断中，申请PendSV。

若Systick申请PendSV后，发生了一个外部中断，则PendSV不会立即执行，而是会等到此外部中断结束后，才会延期执行。外部中断的实时性得到了保证。

仍存在的问题：若外部中断刚好发生在Systick中断执行时，systick有可能被分割，导致节拍的偏差。不过systick中断体本身极小，故影响不大。

## 移植FreeRTOS

### 获取源码

在移植FreeRTOS之前，需要先从[官网](http://www.freertos.org)获取源码。

### 工程目录

- 在工作空间创建文件夹`demo`，在其下创建`FreeRTOS`、`User`、`Project`子文件夹。

- 在刚创建的`FreeRTOS`文件夹下创建`src`、`port`、`inc`子文件夹。

- 将源码`FreeRTOS-Kernel`目录下所有的`c`文件，拷贝至刚创建的`src`文件夹。

  ![image-20221010131632217](/Users/chng/Library/Application Support/typora-user-images/image-20221010131632217.png)

- 将源码`FreeRTOS-Kernel/portable`目录下的`MemMang/heap_4.c`、`RVDS/ARM_CM3/port.c`、`RVDS/ARM_CM3/portmacro.h`，拷贝至刚创建的`port`文件夹。

  ![image-20221010131732149](/Users/chng/Library/Application Support/typora-user-images/image-20221010131732149.png)

- 将源码`FreeRTOS-Kernel/include`目录下的所有文件，拷贝至刚创建的`inc`文件夹。

  ![image-20221010131822922](/Users/chng/Library/Application Support/typora-user-images/image-20221010131822922.png)

- 在`User`目录中创建`main.c`和`FreeRTOSConfig.h`

  ![image-20221010132319730](/Users/chng/Library/Application Support/typora-user-images/image-20221010132319730.png)

### 创建工程

- 打开MDK Keil， 在demo/Project创建工程，选择Cortex m3作为核心，在RTE中添加CMSIS和Startup

  ![image-20221010131941206](/Users/chng/Library/Application Support/typora-user-images/image-20221010131941206.png)

  ![image-20221010132034731](/Users/chng/Library/Application Support/typora-user-images/image-20221010132034731.png)

- 创建`User`、`FreeRTOS/src`、`FreeRTOS/port`分组，将User目录下的文件添加至User分组，将FreeRTOS/src目录下的文件添加至FreeRTOS/src分组，将`port.c`和`heap_4.c`添加至`port`分组

  ![image-20221010132139249](/Users/chng/Library/Application Support/typora-user-images/image-20221010132139249.png)

  ![image-20221010132405642](/Users/chng/Library/Application Support/typora-user-images/image-20221010132405642.png)

- 将以下路径添加至头文件搜索路径：
  - demo/User
  - demo/FreeRTOS/inc
  - demo/FreeRTOS/port
  
  ![image-20221010132510075](/Users/chng/Library/Application Support/typora-user-images/image-20221010132510075.png)

### 修改配置文件

拷贝自示例工程的`FreeRTOSConfig.h`。
具体配置项请参考[官方文档](https://www.freertos.org/a00110.html)。

```C
/*
 * FreeRTOSConfig.h
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* 将FreeRTOS中的关键函数 重定向到 CMSIS的函数名 */
#define vPortSVCHandler SVC_Handler
#define xPortSysTickHandler SysTick_Handler
#define xPortPendSVHandler PendSV_Handler

/* 基本配置 */
#define configUSE_PREEMPTION                             1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION          1
#define configTICK_RATE_HZ                               ( 1000 )												/* 系统节拍频率 */
#define configCPU_CLOCK_HZ															 ( 25000000 )										/* CPU时钟频率 */
#define configSYSTICK_CLOCK_HZ                           ( 25000000 )										/* SysTick频率，若systick频率与CPU时钟频率不一致，可通过此项来指定 */
#define configMAX_PRIORITIES                             ( 7 )													/* 最多可以有几个任务优先级 */
#define configMINIMAL_STACK_SIZE                         ( ( unsigned short ) 128 )			/* 最小任务栈的大小，单位是字（32bits），空闲任务使用的栈便是此大小 */
#define configMAX_TASK_NAME_LEN                          ( 12 )													/* 任务名最长长度 */
#define configUSE_16_BIT_TICKS                           0															/* 使用多少位来表示系统节拍。若设为1，则TickType_t将为16位，最大可表示65535个节拍的延时；若设为0，则为32位。 */
#define configIDLE_SHOULD_YIELD                          1															/* 若有其他任务使用了空闲任务优先级（0），这些任务会和空闲任务一起进行时间片轮询。若此项为1，空闲任务执行完成后会主动挂起，让下一个任务执行，空闲任务和下一个任务会共享一个时间片；若此项为0，空闲任务和其他任务一样都会独享时间片。 */
#define configUSE_MUTEXES                                1															/* 开启互斥量 */
#define configUSE_RECURSIVE_MUTEXES                      1															/* 开启递归互斥量 */
#define configUSE_COUNTING_SEMAPHORES                    1															/* 开启计数信号量 */
#define configUSE_TASK_NOTIFICATIONS                     1															/* 开启任务通知 */
#define configQUEUE_REGISTRY_SIZE                        20															/* 最多允许注册多少个queue，注册过的queue可通过RTOS kernel aware debugger查看到 */
#define configUSE_QUEUE_SETS                             1															/* 允许queue set */
#define configUSE_TIME_SLICING                           1                              /* (缺省为1)开启时间片。开启时间片时，相同优先级的任务会进行时间片轮询调度；若不开启，则不进行时间片轮询调度 */
#define configUSE_NEWLIB_REENTRANT                       0                              /* 使用NEWLIB */
#define configENABLE_BACKWARD_COMPATIBILITY              0															/* 向低版本兼容 */
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS          5															/* THREAD_LOCAL_STORAGE的长度，TLS允许开发者往任务控制块中存一些变量 */
#define configUSE_MINI_LIST_ITEM                         1															/* 设为1，则使用MINI_LIST_ITEM来存放任务列表的首尾，节省一些RAM */
//#define configSTACK_DEPTH_TYPE                           uint16_t												/* 任务栈的位宽 */
#define configHEAP_CLEAR_MEMORY_ON_FREE                  1															/* 通过vPortFree()释放的堆空间将被清零 */

/* 中断、优先级相关 */
#define configKERNEL_INTERRUPT_PRIORITY 								( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 						( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configPRIO_BITS 																	4															/* 优先级位数 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY						15														/* 最低中断优先级 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 			5															/* 最大系统调用优先级：高于此优先级（数值小于）的中断，将不受临界区的限制永远可以被执行，但是不可以调用任何FreeRTOS API！*/

/* 内存分配相关 */
#define configSUPPORT_STATIC_ALLOCATION                  0
#define configSUPPORT_DYNAMIC_ALLOCATION                 1
#define configTOTAL_HEAP_SIZE                            ( ( size_t ) ( 52 * 1024 ) )		
#define configAPPLICATION_ALLOCATED_HEAP                 0															/* 开发者自行分配堆空间，通过uint8_t ucHeap[configTOTAL_HEAP_SIZE]来指定 */
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP        0                              /* 开发者自行为任务栈分配空间 */

/* 软件定时器相关 */
#define configUSE_TIMERS                                 1
#define configTIMER_TASK_PRIORITY                        ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                         10
#define configTIMER_TASK_STACK_DEPTH                     ( configMINIMAL_STACK_SIZE * 2 )

/* 协程相关 */
#define configUSE_CO_ROUTINES                            1
#define configMAX_CO_ROUTINE_PRIORITIES                  ( 2 )

/* 钩子函数、回调函数相关 */
#define configUSE_IDLE_HOOK                              0															/* 开启空闲任务钩子 */
#define configUSE_TICK_HOOK                              0															/* 开启系统节拍钩子 */
#define configUSE_DAEMON_TASK_STARTUP_HOOK               0                              /* RTOS daemon task (timer service stack)启动时钩子 */
#define configUSE_MALLOC_FAILED_HOOK                     0															/* 内存分配失败钩子 */
#define configCHECK_FOR_STACK_OVERFLOW                   0															/* 检查栈溢出 */
#define configUSE_SB_COMPLETED_CALLBACK                  0                              /* stream buffer 相关回调函数 */

/* 统计相关 */
#define configUSE_TRACE_FACILITY                         0
#define configGENERATE_RUN_TIME_STATS    0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0
unsigned long ulGetRunTimeCounterValue( void ); 
void vConfigureTimerForRunTimeStats( void ); 
//#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()    vConfigureTimerForRunTimeStats()
//#define portGET_RUN_TIME_COUNTER_VALUE()            ulGetRunTimeCounterValue()

/* 断言 */
#define configASSERT( x )    if( x == 0 ) {for(;;);}																		/* 用于debugger下调试，断言失败将进入死循环方便定位错误 */
//#define configASSERT( x )    if( x == 0 ) vAssertCalled(__FILE__, __LINE__)
extern void vAssertCalled( unsigned long ulLine, const char * const pcFileName );

/* 其他可选功能 */
#define INCLUDE_vTaskPrioritySet                  1
#define INCLUDE_uxTaskPriorityGet                 0
#define INCLUDE_vTaskDelete                       1
#define INCLUDE_vTaskCleanUpResources             0
#define INCLUDE_vTaskSuspend                      1
#define INCLUDE_vTaskDelayUntil                   1
#define INCLUDE_vTaskDelay                        1
#define INCLUDE_uxTaskGetStackHighWaterMark       0
#define INCLUDE_xTaskGetSchedulerState            1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle    0
#define INCLUDE_xTaskGetIdleTaskHandle            0
#define INCLUDE_xTaskGetHandle                    0
#define INCLUDE_eTaskGetState                     1
#define INCLUDE_xSemaphoreGetMutexHolder          0
#define INCLUDE_xTimerPendFunctionCall            1
#define INCLUDE_xTaskAbortDelay                   0

#endif /* FREERTOS_CONFIG_H */

```



## 源码结构

- User
    - main.c
    - FreeRTOSConfig.h —— 配置文件
- FreeRTOS
    - inc——头文件
    - src
        - croutine.c —— 协程，除非资源受限，否则不推荐使用
        - event_group.c —— 事件，新版本可被task_notification代替
        - list.c —— 任务列表
        - queue.c —— 消息队列
        - tasks.c —— 任务
        - timers.c —— 定时器
    - port
        - (heap_1.c) —— 简单，不可free
        - (heap_2.c) —— 可以free，但无法合并相邻空闲块
        - (heap_3.c) —— 就是标准的malloc和free，但是线程安全
        - heap_4.c —— 可以自动合并相邻空闲块（常用）
        - (heap_5.c) —— 允许堆跨越不相邻的块
        - port.c —— 移植相关
        - portmacro.h —— 移植相关宏

## 上手应用

### 任务

![截屏2022-10-10 上午1.53.11](/Users/chng/Library/Application Support/typora-user-images/截屏2022-10-10 上午1.53.11.png)

- 就绪：等待被调度器调用
- 运行：正在运行
- 阻塞（pend）：任务等待延时到期、等待信号量、等待消息队列、等待事件等
- 挂起（suspend）：长时间不需要运行的任务可以手动挂起，调度器将完全忽视此任务

#### 创建任务

```c
void vTaskCode( void * pvParameters )
{
    for( ;; )
    {
        /* Task code goes here. */
    }
}

void main( void )
{
  BaseType_t xReturned;
  TaskHandle_t xHandle = NULL;
  xReturned = xTaskCreate(
      vTaskCode,       /* 任务入口函数指针 */
      "NAME",          /* 任务名，仅作识别用 */
      STACK_SIZE,      /* 栈大小，单位是字 */
      ( void * ) NULL, /* 传递的参数 */
      tskIDLE_PRIORITY,/* 任务优先级,数值越大优先级越高 */
      &xHandle );      /* 返回的任务句柄 */

  if( xReturned == pdPASS )
  {
      vTaskDelete( xHandle );
  }
}
```

#### 任务挂起与恢复

挂起：

```c
void vTaskSuspend( TaskHandle_t xTaskToSuspend );
```

恢复：

```c
void vTaskResume( TaskHandle_t xTaskToResume );
```

在中断服务程序中，需使用：

```c
BaseType_t xTaskResumeFromISR( TaskHandle_t xTaskToResume );
```

> 注意：如需使用此功能，需在配置中将INCLUDE_vTaskSuspend 设为1

#### 任务延时(阻塞延时)

```c
void vTaskDelay( const TickType_t xTicksToDelay );
```

单位是tick，若要以ms为单位，可使用：

```c
vTaskDelay( time_in_ms / portTICK_PERIOD_MS );
```

#### 任务的设计要点

FreeRTOS中的3种运行上下文：

- 中断服务程序
- 普通任务
- 空闲任务

中断服务程序：不允许阻塞，不允许挂起；处理时间不可过长。

普通任务：不可出现死循环（指的是不会阻塞、挂起，不让出CPU使用权限，低优先级的任务将无法运行）

空闲任务：自动创建，进行内存回收。可以通过「空闲任务钩子」将用户自己的函数插入到空闲任务中执行。空闲任务钩子不允许阻塞、挂起（因为没有优先级更低的任务），不允许进入死循环。

### 消息队列

消息队列可用于在任务间传递消息。

#### 创建消息队列

```c
QueueHandle_t xQueueCreate( UBaseType_t uxQueueLength,
                             UBaseType_t uxItemSize );
```

#### 发送消息

普通任务、中断服务程序都可以往消息队列发送消息，消息将插入至队尾。

在普通任务中，使用：

```c
 BaseType_t xQueueSend(
                            QueueHandle_t xQueue,	//消息队列句柄
                            const void * pvItemToQueue, //指向待发送内容的指针
                            TickType_t xTicksToWait //阻塞等待超时时间
                         );
```

> xQueueSend等效于xQueueSendToBack

在中断服务程序中，使用：

```C
xQueueSendFromISR
```

发送消息时，若队列未满，消息将被拷贝至队尾；

若队列已满，将进入阻塞等待，直至队列空闲或超时（中断服务程序中将直接返回False）。

> 若要发送紧急消息，使用xQueueSendToFront和xQueueSendToFrontFromISR，消息将被插到队首

#### 读取消息

普通任务读取消息：

```C
BaseType_t xQueueReceive(
                               QueueHandle_t xQueue, //队列句柄
                               void *pvBuffer, //读取消息存放位置
                               TickType_t xTicksToWait //阻塞等待超时时间
                            );
```

中断服务程序读取消息：

```C
xQueueReceiveFromISR
```

读取消息时，若队列非空，将队首消息拷贝至*pvBuffer；若队列为空，将阻塞等待，直至队列中有消息或超时（中断服务程序中将直接返回）。

### 信号量

信号量（Semaphore）是一种实现任务间通信的机制，可以实现任务之间同步或临界资
源的互斥访问。

信号量表现为一个非负的计数器，其值可以为0和其他正整数。

「释放」一个信号量，信号量的值将加1；

「获取」一个信号量，若该信号量大于0，将减1；若等于0，将进入阻塞等待，直至大于0，然后再减1.

#### 四种信号量

- 计数信号量
- 二值信号量
- 互斥信号量
- 递归信号量

##### 计数信号量

常见的应用场景：“生产——消费”模型：

- 任务A产生信号量——任务B消费信号量

- 中断I产生信号量——任务B消费信号量（可以保持中断服务程序的短小精简）

创建：

```C
SemaphoreHandle_t xSemaphoreCreateCounting( UBaseType_t uxMaxCount,//最大计数值
                                            UBaseType_t uxInitialCount);//计数初始值
```

释放：

```C
xSemaphoreGive( SemaphoreHandle_t xSemaphore );
//在ISR中，使用xSemaphoreGiveFromISR
```

获取：

```C
 xSemaphoreTake( SemaphoreHandle_t xSemaphore,
                 TickType_t xTicksToWait ); //阻塞等待超时时间
//在ISR中，使用xSemaphoreTakeFromISR
```

##### 二值信号量

二值信号量即计数信号量在uxMaxCount为1的特殊情况，仅可计0和1两个数。

二值信号量常用于任务间的同步或中断与任务的同步。

应用实例：温度传感器与液晶显示屏。

##### 互斥信号量

互斥信号量是一种特殊的二值信号量，其初始化计数值为1。

互斥信号量常用于临界资源的互斥访问：

- 当一个任务要访问临界资源时，对互斥锁上锁，即将信号量设为0，此后访问该资源的任务将进入阻塞等待。
- 访问临界资源结束后，对互斥锁解锁，即将信号量设为1，此时上述阻塞等待的任务获取信号量，能够对临界资源进行访问。

创建互斥信号量：

```C
SemaphoreHandle_t xSemaphoreCreateMutex( void )
```

上锁：

```C
xSemaphoreTake(SemaphoreHandle_t xSemaphore,
                 TickType_t xTicksToWait )
```

解锁：

```C
xSemaphoreGive(SemaphoreHandle_t xSemaphore )
```

##### 递归信号量

递归信号量是一种特殊的互斥信号量，又称递归互斥量。

普通的互斥信号量在上锁的情况下，无法再次上锁（仅能上一把锁）。

而递归信号量在已上锁的情况下，可以由上锁的任务再次上锁（可以上多把锁）。

递归信号量的解锁，也需要将所有的锁都解开，才能访问临界资源。

### 事件

事件是一种实现任务间通信的机制，主要用于实现多任务间的同步，与信号量不同的是，它可以实现一对多，多对多的同步。

事件仅用于告知事件的发生，不负责数据的传输。

事件存储在EventBits_t类型的变量中，它可以是16或32位的，可以由configUSE_16_BIT_TICKS 来配置，若configUSE_16_BIT_TICKS 为0，EventBits_t是32位，其中24位可以用于存储事件组，每一位可以代表一个事件的触发与否。

事件本质上是标识位。在裸机程序中，使用全局变量作标识位，在循环体中不断对标识位进行判断，在满足某些条件时执行相应的操作，这种方式非常浪费CPU计算资源；而在FreeRTOS中，任务可以阻塞等待某个事件的触发（标识位满足某些条件），从而执行相应的操作。

#### 创建事件组

```C
 EventGroupHandle_t xEventGroupCreate( void );
```

#### 触发事件

触发事件即将事件组中的某些（个）位置1.

```C
 EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup,
                                 const EventBits_t uxBitsToSet );
```

#### 等待事件

```C
EventBits_t xEventGroupWaitBits(
                       const EventGroupHandle_t xEventGroup, //事件组句柄
                       const EventBits_t uxBitsToWaitFor, //要等待的事件
                       const BaseType_t xClearOnExit, //是否自动清除
                       const BaseType_t xWaitForAllBits, //若为1，则所有事件均触发才退出阻塞等待；若为0，则只要有一个事件触发就退出阻塞等待。
                       TickType_t xTicksToWait );//阻塞等待超时时间
```

#### 清除事件

```C
 EventBits_t xEventGroupClearBits(
                                 EventGroupHandle_t xEventGroup,
                                 const EventBits_t uxBitsToClear );
```

#### 应用实例

当按键A和按键B均被按下时，输出一行话。

### 直接任务通知

直接任务通知是FreeRTOS 8.2版本号新提供的功能，其具有更高的效率，在一定情况下可以代替消息队列、信号量、事件组。

传统的ICP方式，如消息队列、信号量等，需要创建一块共享的空间。任务在“阻塞等待”中访问这块空间的内容。

而任务通知可以直接向指定任务发送通知值，无需访问“共享空间”，速度可以比传统方式快45%。

FreeRTOS在创建任务时，会在任务控制块TCB中创建一个（或多个）32位宽的通知值，不需要用户手动创建。

#### 任务通知

##### 发送任务通知

```C
BaseType_t xTaskNotify( TaskHandle_t xTaskToNotify,//向哪一个任务发送通知
                       uint32_t ulValue,//通知值
                       eNotifyAction eAction );//指定操作，枚举类型，可用产生一些指定的操作,见下文

BaseType_t xTaskNotifyIndexed( TaskHandle_t xTaskToNotify,//向通知列表中的哪一个通知发送
                              UBaseType_t uxIndexToNotify, 
                              uint32_t ulValue, 
                              eNotifyAction eAction );
```

注意：10.4.0以前的版本，每个任务仅有一个通知值，调用第一个函数可向其发送通知。10.4.0及以后的版本，每个任务可以有多个通知值，构成一个通知列表，其长度可以由configTASK_NOTIFICATION_ARRAY_ENTRIES指定。若使用第一个函数，将向通知列表的第一项发送通知，使用第二个函数，可以由xTaskNotifyIndexed指定向通知列表中的哪一个通知值发送通知。

**eAction有以下可选项：**

- eNoAction：使用此选项，通知值将不会更新，ulValue无意义。
- eSetBits：将ulValue与通知值按位或，即将掩码对应位置1，这种情况可视作**事件组**。
- eIncrement：将通知值加一，等效于xTaskNotifyGive()，ulValue无意义，这种情况可视作**计数信号量**。
- eSetValueWithOverwrite：强制更新通知值。
- eSetValueWithoutOverwrite：若当前存在一个未被接收的通知值，则不会更新；否则，更新。这种情况可视作一个长度为1的**消息队列**。

##### 等待通知

```C
BaseType_t xTaskNotifyWait( uint32_t ulBitsToClearOnEntry,//函数入口将掩码对应位通知清零
                           uint32_t ulBitsToClearOnExit,//函数出口处掩码对应位通知值清零
                           uint32_t *pulNotificationValue,//接收到的通知值存储于此
                           TickType_t xTicksToWait );//阻塞等待超时时间

BaseType_t xTaskNotifyWaitIndexed( UBaseType_t uxIndexToWaitOn, //通知值在列表中的下标
                                  uint32_t ulBitsToClearOnEntry, 
                                  uint32_t ulBitsToClearOnExit, 
                                  uint32_t *pulNotificationValue, 
                                  TickType_t xTicksToWait );
```

调用此函数的任务，将进入阻塞状态，等待通知的更新，直至超时。

若在超时时间内等待到通知，将返回pdPASS（1）

#### 任务通知版信号量

32位的通知值可作为信号量中的计数器，以此实现信号量的功能。

除了上文所述使用`eIncrement`，FreeRTOS还提供了更简单的方式：

##### 释放信号量：

```C
BaseType_t xTaskNotifyGive( TaskHandle_t xTaskToNotify );

BaseType_t xTaskNotifyGiveIndexed( TaskHandle_t xTaskToNotify, 
                                  UBaseType_t uxIndexToNotify );
```

##### 获取信号量

```C
 uint32_t ulTaskNotifyTake( BaseType_t xClearCountOnExit, //是否在退出时清空计数器
                            TickType_t xTicksToWait ); //超时时间
 
 uint32_t ulTaskNotifyTakeIndexed( UBaseType_t uxIndexToWaitOn, 
                                   BaseType_t xClearCountOnExit, 
                                   TickType_t xTicksToWait );
```

### 流和消息缓冲区

流/消息缓冲区，为**单端写入，单端读取**的应用场景，提供了一种任务与任务、中断与任务之间的通信方式。

#### 流缓冲区

- 流缓冲区可以将字节流从一个任务发送到另一个任务，或是从一个中断发送到一个任务。

- 字节流可以是任意的长度，并且可以无头无尾。
- 数据以**拷贝**，而不是引用的方式传输。
- 流缓冲区底层依靠**直接任务通知**来实现，要注意避免冲突。

##### 创建流缓冲区

```C
StreamBufferHandle_t xStreamBufferCreate( size_t xBufferSizeBytes, //缓冲区大小，单位是字节
                                         size_t xTriggerLevelBytes );//当缓冲区中至少有多少字节时，正在阻塞等待的任务能退出阻塞并读取字节流

//使用上面一种方法创建的所有流缓冲区，将公用sbSEND_COMPLETED()和sbRECEIVE_COMPLETED()作为回调函数

StreamBufferHandle_t xStreamBufferCreateWithCallback( //指定回调函数的版本
                         size_t xBufferSizeBytes,
                         size_t xTriggerLevelBytes
                         StreamBufferCallbackFunction_t pxSendCompletedCallback,
                         StreamBufferCallbackFunction_t pxReceiveCompletedCallback );
//使用第二种方法创建的流缓冲区，可以指定单独的回调函数
```

##### 写入流缓冲区

```C
size_t xStreamBufferSend( StreamBufferHandle_t xStreamBuffer,
                          const void *pvTxData, //指向待写入数据的指针
                          size_t xDataLengthBytes, //待写入字节数
                          TickType_t xTicksToWait ); //阻塞等待超时时间
```

在写流缓冲区时，若缓冲区无可用空间，任务将进入阻塞等待，直至可用空间充足或超时。

若在ISR中调用，应使用：

```C
size_t xStreamBufferSendFromISR( StreamBufferHandle_t xStreamBuffer,
                                 const void *pvTxData,
                                 size_t xDataLengthBytes,
                                 BaseType_t *pxHigherPriorityTaskWoken );
```

##### 读取流缓冲区

```C
size_t xStreamBufferReceive( StreamBufferHandle_t xStreamBuffer,
                             void *pvRxData,
                             size_t xBufferLengthBytes,
                             TickType_t xTicksToWait );

size_t xStreamBufferReceiveFromISR( StreamBufferHandle_t xStreamBuffer,
                                    void *pvRxData,
                                    size_t xBufferLengthBytes,
                                    BaseType_t *pxHigherPriorityTaskWoken );
```

#### 消息缓冲区

相较于流缓冲区，传输的基本单位是字节；消息缓冲区以“消息”作为基本单位。消息是可变长的，并且发送和接收端总能保证消息长度的一致，即接收端总能收到完整的消息，而不是一个一个字节。

在发送消息时，系统会使用字节流中的4个字节（32位机）来表示该消息的长度，故实际可用的消息长度是设定的消息缓冲区长度减4。

具体API请参考[官方文档🔗](https://www.freertos.org/RTOS-message-buffer-API.html)

### 定时器

使用定时器可以让任务在未来的某一时刻（或几个时刻）得到执行。

- 定时器任务不使用中断上下文
- 在定时器截止前，不会占用CPU的处理时间
- 不会给systick中断增加额外的负担

