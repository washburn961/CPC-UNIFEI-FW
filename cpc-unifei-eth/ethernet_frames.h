#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ETHERNET_FRAME_MAX_SIZE 1524
#define MAX_TLV_OBJECTS 512

typedef struct
{
	uint8_t memory_pool[ETHERNET_FRAME_MAX_SIZE];
	uint32_t frame_length;
	uint32_t insertion_offset;
} ethernet_frame;

typedef struct
{
	const char* name;
	uint32_t offset;
	uint8_t len_length;
	uint32_t value_length;
	uint8_t is_nested; // Flag to mark if this TLV has nested objects
} tlv_object_handle;

// Function prototypes
void ethernet_frame_init(ethernet_frame* frame);
int ethernet_frame_add_tlv(ethernet_frame* frame, const char* name, uint8_t type, uint32_t length, uint8_t* value);
int ethernet_frame_modify_tlv(ethernet_frame* frame, const char* name, uint8_t type, uint32_t length, uint8_t* value);
int ethernet_frame_remove_tlv(ethernet_frame* frame, const char* name);
int ethernet_frame_add_nested_tlv(ethernet_frame* frame, const char* parent_name, const char* child_name, uint8_t type, uint32_t length, uint8_t* value);
uint32_t ethernet_frame_get_tlv_total_length(const char* name);