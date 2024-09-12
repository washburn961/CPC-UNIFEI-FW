#pragma once

#include <stdint.h>
#include <stddef.h>

#define MAX_ARG_SIZE 128
#define MAX_COMMANDS 16

typedef struct {
	uint8_t id;
	size_t arg_size;
	uint8_t args[MAX_ARG_SIZE];
} message_t;

// Registers a command with a specific ID and handler function
void command_handler_register(uint8_t command_id, void(*handler)(void *args, size_t arg_size));

// Executes the command based on its ID and arguments
void execute_command(uint8_t command_id, void *args, size_t arg_size);

// Parses the received message into a command ID and its arguments
void parse_message(char *msg, message_t *parsed_msg);