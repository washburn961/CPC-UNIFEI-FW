#include "communication.h"
#include <string.h>
#include "udp_server.h"

void communication_task(void *argument);

osThreadId_t communicationTaskHandle;
char received_msg[MAX_UDP_PAYLOAD_SIZE];

const osThreadAttr_t communicationTask_attributes = {
	.name = "communicationTask",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};

void communication_init(void)
{
	communicationTaskHandle = osThreadNew(communication_task, NULL, &communicationTask_attributes);
}

void communication_task(void *argument)
{
	while (1)
	{
		osStatus_t status = osMessageQueueGet(udp_message_queue, received_msg, NULL, osWaitForever);
		if (status == osOK)
		{

		}
	}
}