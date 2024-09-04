#pragma once

#include <stdint.h>

uint32_t application_init(void);
void application_analog_semaphore_release(void);
void application_analog_busy_semaphore_release(void);
uint32_t application_analog_busy_semaphore_can_release(uint16_t GPIO_Pin);

struct ads8686s_conversion_voltage {
	float channel_a;
	float channel_b;
};