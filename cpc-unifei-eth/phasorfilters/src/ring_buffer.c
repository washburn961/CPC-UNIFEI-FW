#include "ring_buffer.h"

// Initializes the ring buffer (assumes content is already allocated)
uint32_t ring_buffer_init(ring_buffer* buffer) {
	buffer->head = 0;
	buffer->tail = 0;
	buffer->is_full = 0;

	for (uint32_t i = 0; i < buffer->size; ++i) {
		buffer->content[i] = 0;
	}

	return 0;
}

// Writes data to the ring buffer
uint32_t ring_buffer_write(ring_buffer* buffer, float data) {
	buffer->content[buffer->tail] = data;

	// Move the tail forward
	buffer->tail = (buffer->tail + 1) % buffer->size;

	// If the buffer is full, move the head forward to overwrite oldest data
	if (buffer->is_full) {
		buffer->head = (buffer->head + 1) % buffer->size;
	}

	// Check if buffer is full
	buffer->is_full = (buffer->tail == buffer->head);

	return 0;
}

// Reads data from the ring buffer at a specified index (reverse order)
uint32_t ring_buffer_read(ring_buffer* buffer, float* out_data, uint32_t index) {
	uint32_t actual_index;

	if (index >= buffer->size) {
		return 1; // Out of bounds
	}

	if (buffer->is_full) {
		actual_index = (buffer->tail + buffer->size - index - 1) % buffer->size;
	}
	else {
		if (index >= buffer->tail) {
			return 1; // Out of bounds (nothing to read)
		}
		actual_index = (buffer->tail - index - 1 + buffer->size) % buffer->size;
	}

	*out_data = buffer->content[actual_index];
	return 0;
}

// Reads multiple elements from the ring buffer
uint32_t ring_buffer_read_many(ring_buffer* buffer, float* out_data, uint32_t out_size) {
	if (out_size > buffer->size) return 1; // Too many elements requested

	for (uint32_t i = 0; i < out_size; ++i) {
		if (ring_buffer_read(buffer, &out_data[i], i) != 0) {
			return 1; // Error reading
		}
	}

	return 0;
}
