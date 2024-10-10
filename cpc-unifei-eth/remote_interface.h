#pragma once

#include <stdint.h>
#include <stddef.h>

#define COMMAND_QUEUE_SIZE 16

#define CONFIG_CMD 0xAB

typedef struct
{
	uint8_t command_id;
	uint8_t* payload;
	size_t len;
} remote_command;

void remote_interface_init(void);
void remote_interface_add_to_queue(uint8_t command_id, uint8_t* payload, size_t len);