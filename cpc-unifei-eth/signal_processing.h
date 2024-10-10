#pragma once

#include "stddef.h"
#include "config.h"

typedef struct
{
	uint8_t channel;
	analog_channel_config* config;
} processing_channel;

void signal_processing_channel_config(uint8_t channel, analog_channel_config* config);
void signal_processing_step(uint8_t channel, float input);
void signal_processing_raw_get(uint8_t channel, float* out);
void signal_processing_real_get(uint8_t channel, size_t index, float* out);
void signal_processing_imag_get(uint8_t channel, size_t index, float* out);