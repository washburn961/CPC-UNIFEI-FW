#include "signal_processing.h"
#include "ring_buffer.h"
#include "dft.h"

#define ONE_OVER_SQRT2 0.70710678118

typedef void(*phasor_filter_execute)(uint8_t channel, float input);
typedef float(*phasor_filter_get_mag)(uint8_t channel, size_t index);
typedef float(*phasor_filter_get_phase)(uint8_t channel, size_t index);

typedef struct
{
	float last_value;
	ring_buffer ring_buffer;
	dft dft;
	phasor_filter_execute filter_exec;
	phasor_filter_get_mag filter_get_mag;
	phasor_filter_get_phase filter_get_phase;
	float real[SAMPLE_COUNT];
	float imag[SAMPLE_COUNT];
	float ring_buffer_content[SAMPLE_COUNT];
	float intermediate_buffer[SAMPLE_COUNT];
} processing_channel_internals;

processing_channel_internals internals[CHANNEL_COUNT] = { 0 };
processing_channel channels[CHANNEL_COUNT] = { 0 };

float sin_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };
float cos_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };

float apply_ratios(uint8_t channel, float input);
void dft_execute(uint8_t channel, float input);
float dft_get_mag(uint8_t channel, size_t index);
float dft_get_ph(uint8_t channel, size_t index);

void signal_processing_channel_config(uint8_t channel, analog_channel_config* config)
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
		channel_internals->filter_get_mag = &dft_get_mag;
		channel_internals->filter_get_phase = &dft_get_ph;
		dft_init(&(channel_internals->dft));
		
	default:
		break;
	}
}

void signal_processing_step(uint8_t channel, float input)
{
	if ((channels[channel].config == NULL) || (channels[channel].config->is_enabled == false)) return;
	
	processing_channel_internals* channel_internals = &(internals[channel]);
	
	channel_internals->last_value = apply_ratios(channel, input);
	
	channel_internals->filter_exec(channel, channel_internals->last_value);
}

void signal_processing_raw_get(uint8_t channel, float* out)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	*out = channel_internals->last_value;
}
void signal_processing_phase_get(uint8_t channel, size_t index, float* out)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	*out = channel_internals->filter_get_phase(channel, index);
}
void signal_processing_mag_get(uint8_t channel, size_t index, float* out)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	*out = channel_internals->filter_get_mag(channel, index) * ONE_OVER_SQRT2;
}
void signal_processing_real_get(uint8_t channel, size_t index, float* out)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	*out = channel_internals->real[index] * ONE_OVER_SQRT2;
}
void signal_processing_imag_get(uint8_t channel, size_t index, float* out)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	*out = channel_internals->imag[index] * ONE_OVER_SQRT2;
}

float apply_ratios(uint8_t channel, float input)
{
	return input * (channels[channel].config->adc_to_sec_ratio) * (channels[channel].config->itr_ratio);
}

void dft_execute(uint8_t channel, float input)
{
	processing_channel_internals* channel_internals = &(internals[channel]);
	
	ring_buffer_write(&(channel_internals->ring_buffer), input);
	ring_buffer_read_many(&(channel_internals->ring_buffer), channel_internals->intermediate_buffer, SAMPLE_COUNT);
	dft_step(&(channel_internals->dft), channel_internals->intermediate_buffer);
}

float dft_get_mag(uint8_t channel, size_t index)
{
	float tmp;
	processing_channel_internals* channel_internals = &(internals[channel]);
	
	dft_get_magnitude(&channel_internals->dft, &tmp, index);
	
	return tmp;
}

float dft_get_ph(uint8_t channel, size_t index)
{
	float tmp;
	processing_channel_internals* channel_internals = &(internals[channel]);
	
	dft_get_phase(&channel_internals->dft, &tmp, index);
	
	return tmp;
}