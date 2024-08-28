#include "command_system.h"
#include <stdio.h>
#include <string.h>
#include "command_system_interface.h"

// Function to execute a command based on input string and its length
void execute_command(char *input, unsigned int input_length) {
	char command_name[20] = { 0 };
	char params[50] = { 0 };

	// Use sscanf to extract the command and the parameters
	int ret = sscanf(input, "%19s %49[^\n]", command_name, params);

	// ret will tell us how many items were successfully parsed
	if (ret == 1 || ret == 2) {
		if (strcmp(command_name, "writevalue") == 0) {
			write_value(params, address_map_size); // Pass the size of address_map
		}
		else if (strcmp(command_name, "readvalue") == 0) {
			read_value(params, address_map_size); // Pass the size of address_map
		}
		else {
			printf("Command not found: %s\n", command_name);
		}
	}
	else {
		printf("Invalid command format\n");
	}
}

// Function to handle 'writevalue' command
void write_value(char *params, unsigned int address_map_size) {
	char id[20];
	int value;
    
	sscanf(params, "%s %d", id, &value);

	for (unsigned int i = 0; i < address_map_size; i++) {
		if (strcmp(id, address_map[i].id) == 0) {
			address_map[i].write(value);
			return;
		}
	}
    
	printf("Invalid ID: %s\n", id);
}

// Function to handle 'readvalue' command
void read_value(char *params, unsigned int address_map_size) {
	char id[20];
	sscanf(params, "%s", id);

	for (unsigned int i = 0; i < address_map_size; i++) {
		if (strcmp(id, address_map[i].id) == 0) {
			int value = address_map[i].read();
			printf("%s value: %d\n", id, value);
			return;
		}
	}

	printf("Invalid ID: %s\n", id);
}
