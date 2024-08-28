#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

#include <stdint.h>

// Function prototypes
void execute_command(char *input, unsigned int input_length);
void write_value(char *params, unsigned int address_map_size);
void read_value(char *params, unsigned int address_map_size);

#endif // COMMAND_SYSTEM_H
