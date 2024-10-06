#pragma once

#include <stdint.h>

void time_critical_init(void);
void time_critical_adc_timing_flag_set(void);
void time_critical_adc_busy_flag_set(void);
uint32_t time_critical_can_set_adc_busy_flag(uint16_t GPIO_Pin);