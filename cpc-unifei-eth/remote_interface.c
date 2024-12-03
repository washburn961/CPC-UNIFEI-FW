#include "remote_interface.h"
#include "cmsis_os.h"
#include "debug.h"
#include "config.h"
#include <string.h>
#include <stdlib.h>

void remote_interface_task(void *argument);
void handle_config_command(remote_command* cmd);

osMessageQueueId_t remote_command_queue;
osThreadId_t remote_interface_task_handle;
const osThreadAttr_t remote_interface_task_attributes = {
	.name = "remote_interface_task",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};

void remote_interface_init(void)
{
	remote_command_queue = osMessageQueueNew(COMMAND_QUEUE_SIZE, sizeof(remote_command), NULL);
	
	if (remote_command_queue == NULL)
	{
		DEBUG_ERROR("Failed to create remote command queue.\r\n");
	}
	
	remote_interface_task_handle = osThreadNew(remote_interface_task, NULL, &remote_interface_task_attributes);
}

void remote_interface_add_to_queue(uint8_t command_id, uint8_t* payload, size_t len)
{
	remote_command cmd;
	cmd.command_id = command_id;
	cmd.payload = payload;
	cmd.len = len;

	osMessageQueuePut(remote_command_queue, &cmd, 0, 0);
}

void remote_interface_task(void *argument)
{
	remote_command cmd;
	
	while (1)
	{
		osMessageQueueGet(remote_command_queue, &cmd, NULL, osWaitForever);
		
		switch (cmd.command_id)
		{
		case CONFIG_CMD:
			handle_config_command(&cmd);
			break;

		default:
			DEBUG_WARN("Unknown command ID: %d\r\n", cmd.command_id);
			break;
		}
	}
}

void handle_config_command(remote_command* cmd)
{
	if (cmd->payload == NULL) {
		DEBUG_ERROR("Payload pointer is NULL.\r\n");
		return;
	}
	
	if (cmd->len != sizeof(general_config)) {
		DEBUG_ERROR("Payload size is too small for general_config.\r\n");
		return;
	}
	
	general_config config;
	memcpy(&config, cmd->payload, sizeof(general_config));
	config_set(&config);
	
	free(cmd->payload);
	cmd->payload = NULL;
}