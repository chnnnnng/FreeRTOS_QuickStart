#include "FreeRTOS.h"
#include "task.h"

int flag1 = 0;
int flag2 = 0;

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 1 : 创建两个任务并调度
#if 0
void task1(void * p){
	for(;;){
		flag1 = !flag1;
		vTaskDelay(2);
	}
}


void task2(void * p){
	for(;;){
		flag2 = !flag2;
		vTaskDelay(3);
	}
}

int main(void){
	
	TaskHandle_t * task1handler, * task2handler;
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 2 : 消息队列
#if 0
#include "queue.h"

QueueHandle_t queue;

void task1(void * p){
	flag1 = 0;
	for(;;){
		xQueueSend(queue, &flag1, 0);
		flag1++;
		vTaskDelay(5);
	}
}


void task2(void * p){
	int buf;
	for(;;){
		xQueueReceive(queue, &buf, 6);
		flag2 = buf;
		//vTaskDelay(10);  //此行可用于模拟接受速度比发送速度低的情况
	}
}

int main(void){
	queue = xQueueCreate(10, sizeof(int));
	TaskHandle_t * task1handler, * task2handler;
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 3 : 计数信号量
//观察到现象：task1快速释放5个信号量，task2获取5个信号量
#if 0
#include "semphr.h"
SemaphoreHandle_t sem;

void task1(void * p){
	flag1 = 0;
	int i = 0;
	for(;;){
		for(i=0;i<5;i++){
			xSemaphoreGive(sem);
			flag1++;
			vTaskDelay(5);
		}
		flag1=0;
		vTaskDelay(100);
	}
}


void task2(void * p){
	flag2 = 0;
	for(;;){
		xSemaphoreTake(sem,200);
		flag2++;
		vTaskDelay(10);
	}
}

int main(void){
	sem = xSemaphoreCreateCounting(10, 0);
	
	TaskHandle_t * task1handler, * task2handler;
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 4 : 二值信号量
#if 0
#include "semphr.h"
SemaphoreHandle_t sem;
int temp = 0;
void task1(void * p){
	flag1 = 0;
	int i = 0;
	for(;;){
		xSemaphoreGive(sem);
		flag1 = 1 - flag1;
		temp++;
		vTaskDelay(100);
	}
}


void task2(void * p){
	flag2 = 0;
	for(;;){
		xSemaphoreTake(sem,200);
		flag2 = temp;
	}
}

int main(void){
	sem = xSemaphoreCreateBinary();
	
	TaskHandle_t * task1handler, * task2handler;
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 5 : 互斥信号量
//现象：task1、task2两个任务互斥访问临界资源pub
#if 0
#include "semphr.h"
SemaphoreHandle_t sem;
int pub = 0;
void delay(int t){
	while(t--){}
}

void task1(void * p){
	for(;;){
		xSemaphoreTake(sem, 50000);
		flag1 = pub;
		pub++;
		delay(100000);
		xSemaphoreGive(sem);
		vTaskDelay(100);
	}
}


void task2(void * p){
	for(;;){
		xSemaphoreTake(sem, 50000);
		flag2 = pub;
		pub++;
		delay(100000);
		xSemaphoreGive(sem);
		vTaskDelay(100);
	}
}

int main(void){
	sem = xSemaphoreCreateMutex();
	
	TaskHandle_t * task1handler, * task2handler;
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 6 : 事件
//现象：当flag1是3的倍数、flag2是4的倍数时，触发事件
#if 0
#include "event_groups.h"

EventGroupHandle_t event_handle;
#define EVENT1 (1 << 0)
#define EVENT2 (1 << 1)
void task1(void * p){
	flag1 = 1;
	flag2 = 1;
	EventBits_t bits;
	for(;;){
		flag1++;
		flag2++;
		bits=0;
		xEventGroupClearBits(event_handle, EVENT1|EVENT2);
		if(flag1 % 3 == 0) bits |= EVENT1;
		if(flag2 % 4 == 0) bits |= EVENT2;
		xEventGroupSetBits(event_handle, bits);
		vTaskDelay(20);
	}
}


void task2(void * p){
	for(;;){
		xEventGroupWaitBits(event_handle, EVENT1|EVENT2, pdTRUE, pdTRUE, 50000);
		flag1 = 1;
		flag2 = 1;
	}
}

int main(void){
	event_handle = xEventGroupCreate();
	
	TaskHandle_t * task1handler, * task2handler;
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 7 : 直接任务通知
#if 0

TaskHandle_t task1handler, task2handler;

void task1(void * p){
	uint32_t temp;
	BaseType_t ret;
	for(;;){
		ret = xTaskNotifyWait(0x00, 0x00, &temp, 30);
		if(ret) flag1 = temp;
	}
}


void task2(void * p){
	for(;;){
		xTaskNotify(task1handler, flag2++, eSetValueWithOverwrite);
		vTaskDelay(20);
	}
}

int main(void){
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, &task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, &task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	return 0;
}
#endif


//////////////////////////////////////////////////////////////////////////////////////
//DEMO 8 : 直接任务通知实现信号量
#if 0

TaskHandle_t task1handler, task2handler;

void task1(void * p){
	uint32_t temp;
	for(;;){
		temp = ulTaskNotifyTake(pdFALSE, 500);
		if(temp) flag1++;
		vTaskDelay(50);
	}
}

void task2(void * p){
	int i;
	for(;;){
		for(i=0; i<5; i++){
			xTaskNotifyGive(task1handler);
			flag2++;
			vTaskDelay(20);
		}
		flag2=0;
		vTaskDelay(200);
	}
}

int main(void){
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, &task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, &task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 9 : 直接任务通知实现事件
#if 0

TaskHandle_t task1handler, task2handler;

#define EVENT1 (1 << 0)
#define EVENT2 (1 << 1)

void task1(void * p){
	uint32_t i,temp;
	for(;;){
		for(i=0;i<3;i++){
			for(temp=0;temp<20;temp++){
				flag1++;
				vTaskDelay(10);
			}
			xTaskNotify(task2handler, EVENT1, eSetBits);//触发事件1
			for(temp=0;temp<30;temp++){
				flag1++;
				vTaskDelay(10);
			}
			xTaskNotify(task2handler, EVENT2, eSetBits);//触发事件2
		}
		vTaskDelete(NULL);
	}
}

void task2(void * p){
	uint32_t noti;
	BaseType_t ret;
	for(;;){
		ret = xTaskNotifyWait(0x0, 0x0, &noti, 10);
		if(ret == pdPASS){
			if( (noti & EVENT1) && (noti & EVENT2) ){ //当事件1和事件2均被触发时，执行操作
				flag2 = 1-flag2;
				ulTaskNotifyValueClear(task2handler, EVENT1|EVENT2);//清除标志位
			}
		}
	}
}

int main(void){
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, &task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, &task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 10 : 直接任务通知+中断+消息
//现象：在NVIC中手动将IRQ0设为Pending，观察flag1的变化。
#if 0
#include "RTE_Components.h"                 /* This mechanism is the standard way to include */
#include CMSIS_device_header                /* the device header file, in this case ARMCM33.h */

TaskHandle_t task1handler;
uint32_t active = 0;

void Interrupt0_Handler(){
	active = NVIC_GetActive(Interrupt0_IRQn);
	xTaskNotifyFromISR(task1handler, active, eSetValueWithOverwrite, NULL);
	/* 注意：在FreeRTOSConfig.h中定义了宏configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY，表示了最大系统调用（调用系统API）优先级
	*	高于此优先级（数值小于）的中断，将不受临界区的限制，永远可以被执行，
	* 但是！在此类中断中，将不可以调用任何FreeRTOS API！
	* 在此demo中，MAX_SYSCALL_INTERRUPT_PRIORITY为5，IRQ0的优先级为7，低于最大系统调用优先级，故可以发送任务通知。
	* 若把IRQ0的优先级设为0、1、2、3、4，5，观察现象，会发现中断能被响应，但是任务通知无法发送。
	*/
}


void task1(void * p){
	BaseType_t ret;
	uint32_t temp;
	for(;;){
		ret = xTaskNotifyWait(0x00, 0x00, &temp, 30);
		if(ret) flag1 = 1 - flag1;
	}
}

int main(void){
	NVIC_SetPriority(Interrupt0_IRQn, 7);
	NVIC_EnableIRQ(Interrupt0_IRQn);
	
	BaseType_t ret = pdPASS;
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, &task1handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	
	return 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////
//DEMO 11 :  消息缓冲区
//现象：task1周期性发送消息1，11，22，33，44，55，66，77，88，99，task2能接收到消息并显示。 
#if 1
#include "message_buffer.h"

TaskHandle_t task1handler, task2handler;
MessageBufferHandle_t msg;

void task1(void * p){
	const int data[10] = {1,11,22,33,44,55,66,77,88,99};
	for(;;){
		xMessageBufferSend(msg, data, 10*sizeof(int), 20);
		flag1++;
		vTaskDelay(200);
	}
}

void task2(void * p){
	int data[10];
	size_t ret;
	int i;
	for(;;){
		ret = xMessageBufferReceive(msg, data, 10*sizeof(int), 20);
		if(ret == 10*sizeof(int)){
			for(i=0;i<10;i++) {
				flag2 = data[i];
				vTaskDelay(20);
			}
		}else{
			flag2 = 999;
		}
	}
}

int main(void){
	BaseType_t ret = pdPASS;
	msg = xMessageBufferCreate(20*sizeof(int));
	configASSERT(msg != NULL);
	ret &= xTaskCreate(&task1, "task1", 512, NULL, 2, &task1handler);
	ret &= xTaskCreate(&task2, "task2", 512, NULL, 2, &task2handler);
	if(ret == pdPASS)
		vTaskStartScheduler();
	return 0;
}
#endif