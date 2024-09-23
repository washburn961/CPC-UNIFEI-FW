#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#define _USE_MATH_DEFINES

#include <math.h>
#include <stdint.h>

typedef struct
{
    float* content;
    uint32_t size;

    uint32_t head;
    uint32_t tail;
    uint32_t is_full;
} ring_buffer;

uint32_t ring_buffer_init(ring_buffer* buffer);
uint32_t ring_buffer_write(ring_buffer* buffer, float data);
uint32_t ring_buffer_read(ring_buffer* buffer, float* out_data, uint32_t index);
uint32_t ring_buffer_read_many(ring_buffer* buffer, float* out_data, uint32_t out_size);

#endif