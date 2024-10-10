#include "application.h"
#include <stdio.h>
#include "debug.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "spi.h"
#include "gpio.h"
#include "tim.h"
#include "cmsis_os.h"
#include "ads8686s.h"
#include "udp_server.h"
#include "gpio.h"
#include "ring_buffer.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip.h"
#include "nor_flash.h"

void application_task(void *argument);

osMutexId_t application_mutex_handle;
const osMutexAttr_t application_mutex_attributes = {
	.name = "application_mutex"
};
osThreadId_t application_task_handle;
const osThreadAttr_t application_task_attributes = {
	.name = "application_task",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};

void application_init(void)
{
	application_mutex_handle = osMutexNew(&application_mutex_attributes);
	application_task_handle = osThreadNew(application_task, NULL, &application_task_attributes);
}

void application_task(void *argument)
{
	
	while (true)
	{
//		application_take();
		
		HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
		osDelay(500);
		
//		application_release();
	}
}

void application_take(void)
{
	osMutexAcquire(application_mutex_handle, osWaitForever);
}
void application_release(void)
{
	osMutexRelease(application_mutex_handle);
}