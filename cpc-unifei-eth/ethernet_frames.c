#include "ethernet_frames.h"

tlv_object_handle tlv_objects_pool[MAX_TLV_OBJECTS];
static uint32_t tlv_count = 0; // Track the number of active TLV objects

// Helper to update parent TLVs recursively
static void update_parent_lengths(const char* name, int32_t size_change) {
    for (uint32_t i = 0; i < tlv_count; i++) {
        tlv_object_handle* handle = &tlv_objects_pool[i];
        if (strcmp(handle->name, name) == 0 && handle->is_nested) {
            // Update the length of the current TLV and propagate upwards
            handle->value_length += size_change;
            if (handle->is_nested) {
                update_parent_lengths(handle->name, size_change);
            }
        }
    }
}

// Initialize the Ethernet frame
void ethernet_frame_init(ethernet_frame* frame) {
    if (!frame) return;

    frame->frame_length = 0;
    frame->insertion_offset = 0;
    tlv_count = 0; // Reset the TLV count when initializing the frame
}

// Add a new TLV object to the Ethernet frame
int ethernet_frame_add_tlv(ethernet_frame* frame, const char* name, uint8_t type, uint32_t length, uint8_t* value) { 
	// Ensure the frame, name are valid and TLV count is within limits
	if (!frame || !name || tlv_count >= MAX_TLV_OBJECTS) return -1;

	// Calculate total length (1 byte for type, variable length for value)
	uint32_t len_length = (length < 128) ? 1 : 
	                      ((length <= 255) ? 2 : 
	                      ((length <= 65535) ? 3 : 4));

	uint32_t total_len = 1 + len_length + length; // 1 byte for type + length field + value length

	// Check if there's enough space in the frame
	if (frame->frame_length + total_len > ETHERNET_FRAME_MAX_SIZE) {
		return -1;  // Not enough space
	}

	// Initialize the TLV handle in the pool
	tlv_object_handle* handle = &tlv_objects_pool[tlv_count];
	handle->offset = frame->insertion_offset;
	handle->name = name;
	handle->len_length = len_length;
	handle->value_length = length;
	handle->is_nested = 0; // Default to non-nested

	// Insert type
	frame->memory_pool[frame->insertion_offset++] = type;

	// Insert length
	if (len_length == 1) {
		frame->memory_pool[frame->insertion_offset++] = (uint8_t)length;
	}
	else if (len_length == 2) {
		frame->memory_pool[frame->insertion_offset++] = 0x81; // Long form with 1 extra byte
		frame->memory_pool[frame->insertion_offset++] = (uint8_t)length;
	}
	else if (len_length == 3) {
		frame->memory_pool[frame->insertion_offset++] = 0x82; // Long form with 2 extra bytes
		frame->memory_pool[frame->insertion_offset++] = (uint8_t)((length >> 8) & 0xFF);
		frame->memory_pool[frame->insertion_offset++] = (uint8_t)(length & 0xFF);
	}
	else {
		frame->memory_pool[frame->insertion_offset++] = 0x83; // Long form with 3 extra bytes
		frame->memory_pool[frame->insertion_offset++] = (uint8_t)((length >> 16) & 0xFF);
		frame->memory_pool[frame->insertion_offset++] = (uint8_t)((length >> 8) & 0xFF);
		frame->memory_pool[frame->insertion_offset++] = (uint8_t)(length & 0xFF);
	}

	// Insert value if present and length > 0
	if (value != NULL && length > 0) {
		memcpy(&frame->memory_pool[frame->insertion_offset], value, length);
		frame->insertion_offset += length;
	}

	// Update frame length and TLV count
	frame->frame_length += total_len;
	tlv_count++;

	return 0;
}


// Modify an existing TLV object
int ethernet_frame_modify_tlv(ethernet_frame* frame, const char* name, uint8_t type, uint32_t length, uint8_t* value) {
    if (!frame || !name || !value) return -1;

    // Find the TLV by name
    for (uint32_t i = 0; i < tlv_count; i++) {
        tlv_object_handle* handle = &tlv_objects_pool[i];
        if (strcmp(handle->name, name) == 0) {
            uint32_t old_value_len = handle->value_length;

            if (length > old_value_len) {
                uint32_t additional_len = length - old_value_len;

                if (frame->frame_length + additional_len > ETHERNET_FRAME_MAX_SIZE) {
                    return -1;  // Not enough space
                }

                // Copy new value and update the frame
                memcpy(&frame->memory_pool[handle->offset + 1 + handle->len_length], value, length);
                frame->frame_length += additional_len;
                frame->insertion_offset += additional_len;
            }
            else {
                // Overwrite existing value
                memcpy(&frame->memory_pool[handle->offset + 1 + handle->len_length], value, length);
            }

            // Update the handle's value length
            int32_t size_change = length - old_value_len;
            handle->value_length = length;

            // Update all parents
            if (size_change != 0 && handle->is_nested) {
                update_parent_lengths(handle->name, size_change);
            }

            return 0;
        }
    }

    return -1;  // TLV object not found
}

// Add a nested TLV object within an existing TLV object
int ethernet_frame_add_nested_tlv(ethernet_frame* frame, const char* parent_name, const char* child_name, uint8_t type, uint32_t length, uint8_t* value) {
    if (!frame || !parent_name || !child_name || !value) return -1;

    // Find the parent TLV object by name
    for (uint32_t i = 0; i < tlv_count; i++) {
        tlv_object_handle* parent = &tlv_objects_pool[i];
        if (strcmp(parent->name, parent_name) == 0) {
            // Calculate the size change needed
            uint32_t nested_total_len = 1 + (length < 128 ? 1 : 
                                              (length <= 255 ? 2 : 
                                              (length <= 65535 ? 3 : 4)))
                                       + length;

            if (frame->frame_length + nested_total_len > ETHERNET_FRAME_MAX_SIZE) {
                return -1;  // Not enough space
            }

            // Add the nested TLV inside the parent's value area
            uint32_t nested_offset = parent->offset + 1 + parent->len_length + parent->value_length;

            // Insert type of the nested TLV
            frame->memory_pool[nested_offset++] = type;

            // Insert length of the nested TLV
            uint32_t nested_len_length = (length < 128) ? 1 : 
                                         ((length <= 255) ? 2 : 
                                         ((length <= 65535) ? 3 : 4));

            if (nested_len_length == 1) {
                frame->memory_pool[nested_offset++] = (uint8_t)length;
            } else if (nested_len_length == 2) {
                frame->memory_pool[nested_offset++] = 0x81;  // Long form with 1 extra byte
                frame->memory_pool[nested_offset++] = (uint8_t)length;
            } else if (nested_len_length == 3) {
                frame->memory_pool[nested_offset++] = 0x82;  // Long form with 2 extra bytes
                frame->memory_pool[nested_offset++] = (uint8_t)((length >> 8) & 0xFF);
                frame->memory_pool[nested_offset++] = (uint8_t)(length & 0xFF);
            } else {
                frame->memory_pool[nested_offset++] = 0x83;  // Long form with 3 extra bytes
                frame->memory_pool[nested_offset++] = (uint8_t)((length >> 16) & 0xFF);
                frame->memory_pool[nested_offset++] = (uint8_t)((length >> 8) & 0xFF);
                frame->memory_pool[nested_offset++] = (uint8_t)(length & 0xFF);
            }

            // Insert value of the nested TLV
            memcpy(&frame->memory_pool[nested_offset], value, length);

            // Update parent's value length and mark it as nested
            parent->value_length += nested_total_len;
            parent->is_nested = 1;

            // Update the frame length
            frame->frame_length += nested_total_len;
            frame->insertion_offset += nested_total_len;

            // Update all parent TLVs if this is part of a nested hierarchy
            update_parent_lengths(parent->name, nested_total_len);

            return 0;
        }
    }

    return -1;  // Parent TLV object not found
}

// Remove a TLV object from the Ethernet frame
int ethernet_frame_remove_tlv(ethernet_frame* frame, const char* name) {
	if (!frame || !name) return -1;

	for (uint32_t i = 0; i < tlv_count; i++) {
		tlv_object_handle* handle = &tlv_objects_pool[i];
		if (strcmp(handle->name, name) == 0) {
			uint32_t total_size = 1 + handle->len_length + handle->value_length;

			// Shift memory to remove the TLV
			memmove(&frame->memory_pool[handle->offset],
				&frame->memory_pool[handle->offset + total_size],
				frame->frame_length - (handle->offset + total_size));

			// Adjust the frame length and insertion offset
			frame->frame_length -= total_size;
			frame->insertion_offset -= total_size;

			// Remove the TLV from the handle pool and shift remaining handles
			memmove(&tlv_objects_pool[i], &tlv_objects_pool[i + 1], (tlv_count - i - 1) * sizeof(tlv_object_handle));
			tlv_count--;

			// Update all parent TLVs' lengths, if this TLV was nested
			if (handle->is_nested) {
				update_parent_lengths(handle->name, -((int32_t)total_size));
			}

			return 0;
		}
	}

	return -1;  // TLV object not found
}

// Get the total length of a TLV object
uint32_t ethernet_frame_get_tlv_total_length(const char* name) {
	if (!name) return 0;

	// Search for the TLV object by name
	for (uint32_t i = 0; i < tlv_count; i++) {
		tlv_object_handle* handle = &tlv_objects_pool[i];
		if (strcmp(handle->name, name) == 0) {
			// The total length includes the type byte, length field, and value length
			uint32_t total_length = 1; // 1 byte for the type field

			// Add the length of the length field (len_length) and the value length
			total_length += handle->len_length + handle->value_length;

			return total_length;
		}
	}

	// If the TLV object was not found, return 0
	return 0;
}