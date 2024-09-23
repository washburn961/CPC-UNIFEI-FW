#include "commands.h"
#include <string.h>
#include <stdio.h>

typedef struct {
	uint8_t command_id;
	void(*handler)(void *args, size_t arg_size);
} command_t;

static command_t command_registry[MAX_COMMANDS];
static uint8_t registered_command_count = 0;

// Registers a command with its ID and handler
void command_handler_register(uint8_t command_id, void(*handler)(void *args, size_t arg_size)) {
	if (registered_command_count < MAX_COMMANDS) {
		command_registry[registered_command_count].command_id = command_id;
		command_registry[registered_command_count].handler = handler;
		registered_command_count++;
	}
}

// Function to find and execute the handler for a given command ID
void execute_command(uint8_t command_id, void *args, size_t arg_size) {
	for (uint8_t i = 0; i < registered_command_count; i++) {
		if (command_registry[i].command_id == command_id) {
			if (command_registry[i].handler != NULL) {
				command_registry[i].handler(args, arg_size);
			}
			return;
		}
	};
}

// Parses a message and extracts the command ID and its arguments
void parse_message(char *msg, message_t *parsed_msg) {
	// Assume msg is structured with an ID followed by arguments, e.g., |ID|ARGUMENTS|
	// Parse the message into the message_t structure (you can customize this format)
	parsed_msg->id = msg[0]; // First byte is the command ID
	parsed_msg->arg_size = strlen(msg) - 1; // Everything after the ID is argument data
	memcpy(parsed_msg->args, &msg[1], parsed_msg->arg_size); // Copy the arguments
}