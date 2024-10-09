#pragma once

#include "stddef.h"
#include "config.h"

typedef struct
{
	analog_channel channel;
	analog_channel_config* config;
} processing_channel;

void signal_processing_channel_config(analog_channel channel, analog_channel_config* config);
void signal_processing_step(analog_channel channel, float input);
void signal_processing_raw_get(analog_channel channel, float* out);
void signal_processing_real_get(analog_channel channel, size_t index, float* out);
void signal_processing_imag_get(analog_channel channel, size_t index, float* out);