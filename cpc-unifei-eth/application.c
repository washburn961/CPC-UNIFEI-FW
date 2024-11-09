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

osThreadId_t application_task_handle;
const osThreadAttr_t application_task_attributes = {
	.name = "application_task",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};

void application_init(void)
{
	application_task_handle = osThreadNew(application_task, NULL, &application_task_attributes);
}

void application_task(void *argument)
{
	while (true)
	{
		HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
		osDelay(500);
	}
}