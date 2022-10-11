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
