#include "signal_processing.h"
#include "ring_buffer.h"
#include "dft.h"

typedef void(*phasor_filter_execute)(analog_channel channel, float input);

typedef struct
{
	float last_value;
	ring_buffer ring_buffer;
	dft dft;
	phasor_filter_execute filter_exec;
	float real[SAMPLE_COUNT];
	float imag[SAMPLE_COUNT];
	float ring_buffer_content[SAMPLE_COUNT];
	float intermediate_buffer[SAMPLE_COUNT];
} processing_channel_internals;

processing_channel_internals internals[CHANNEL_COUNT] = { 0 };
processing_channel channels[CHANNEL_COUNT] = { 0 };

float sin_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };
float cos_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };

float apply_ratios(analog_channel channel, float input);
void dft_execute(analog_channel channel, float input);

void signal_processing_channel_config(analog_channel channel, analog_channel_config* config)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	
	channels[channel].channel = channel;
	channels[channel].config = config;
	
	channel_internals->ring_buffer.content = channel_internals->ring_buffer_content;
	channel_internals->ring_buffer.size = SAMPLE_COUNT;
	ring_buffer_init(&(channel_internals->ring_buffer));
	
	switch (config->filter)
	{
	case DFT:
		channel_internals->dft.real = channel_internals->real;
		channel_internals->dft.imag = channel_internals->imag;
		channel_internals->dft.sin = sin_coef;
		channel_internals->dft.cos = cos_coef;
		channel_internals->dft.size = SAMPLE_COUNT;
		channel_internals->filter_exec = &dft_execute;
		dft_init(&(channel_internals->dft));
		
	default:
		break;
	}
}

void signal_processing_step(analog_channel channel, float input)
{
	if ((channels[channel].config == NULL) || (channels[channel].config->is_enabled == false)) return;
	
	processing_channel_internals* channel_internals = &(internals[channel]);
	
	channel_internals->last_value = apply_ratios(channel, input);
	
	channel_internals->filter_exec(channel, channel_internals->last_value);
}

void signal_processing_raw_get(analog_channel channel, float* out)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	*out = channel_internals->last_value;
}
void signal_processing_real_get(analog_channel channel, size_t index, float* out)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	*out = channel_internals->real[index];
}
void signal_processing_imag_get(analog_channel channel, size_t index, float* out)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	*out = channel_internals->imag[index];
}

float apply_ratios(analog_channel channel, float input)
{
	return input * (channels[channel].config->adc_to_sec_ratio) * (channels[channel].config->itr_ratio);
}

void dft_execute(analog_channel channel, float input)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	
	ring_buffer_write(&(channel_internals->ring_buffer), input);
	ring_buffer_read_many(&(channel_internals->ring_buffer), channel_internals->intermediate_buffer, SAMPLE_COUNT);
	dft_step(&(channel_internals->dft), channel_internals->intermediate_buffer);
}