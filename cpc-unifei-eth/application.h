#pragma once

#include <stdint.h>

uint32_t application_init(void);
void application_adc_timing_flag_set(void);
void application_adc_busy_flag_set(void);
void application_goose_flag_set(void);
uint32_t application_analog_busy_semaphore_can_release(uint16_t GPIO_Pin);

struct ads8686s_conversion_voltage {
	float channel_a;
	float channel_b;
};