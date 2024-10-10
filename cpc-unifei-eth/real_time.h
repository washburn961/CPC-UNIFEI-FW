#pragma once

#include <stdint.h>

void real_time_init(void);
void real_time_adc_timing_flag_set(void);
void real_time_adc_busy_flag_set(void);
uint32_t real_time_analog_busy_semaphore_can_release(uint16_t GPIO_Pin);
void real_time_take(void);
void real_time_release(void);