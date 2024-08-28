#ifndef COMMAND_SYSTEM_INTERFACE_H
#define COMMAND_SYSTEM_INTERFACE_H

#include <stdint.h>

// Function pointer types for write and read functions
typedef void(*WriteFunction)(int value);
typedef int(*ReadFunction)(void);

// Structure to map virtual addresses to functions
typedef struct {
	const char *id;
	WriteFunction write;
	ReadFunction read;
} VirtualAddress;

// External declaration of the address map and its size
extern VirtualAddress address_map[];
extern const unsigned int address_map_size;

#endif // COMMAND_SYSTEM_INTERFACE_H
