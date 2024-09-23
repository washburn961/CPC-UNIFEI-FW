#pragma once

#include <stdint.h>

uint32_t ads8686s_low_level_conversion_start(void);
uint32_t ads8686s_low_level_conversion_done(void);
uint32_t ads8686s_low_level_transfer_done(void);
extern uint32_t ads8686s_low_level_transfer_done_callback(uint16_t *buffer);