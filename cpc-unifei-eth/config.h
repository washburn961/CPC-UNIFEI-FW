#pragma once

#include "stdint.h"
#include "stdbool.h"

#define CHANNEL_COUNT 16
#define SAMPLE_COUNT 16
#define BAY_COUNT 5

typedef enum
{
	CHANNEL_0A = 0,
	CHANNEL_0B = 1,
	CHANNEL_1A = 2,
	CHANNEL_1B = 3,
	CHANNEL_2A = 4,
	CHANNEL_2B = 5,
	CHANNEL_3A = 6,
	CHANNEL_3B = 7,
	CHANNEL_4A = 8,
	CHANNEL_4B = 9,
	CHANNEL_5A = 10,
	CHANNEL_5B = 11,
	CHANNEL_6A = 12,
	CHANNEL_6B = 13,
	CHANNEL_7A = 14,
	CHANNEL_7B = 15,
} analog_channel;

typedef enum
{
	VOLTAGE = 0,
	CURRENT = 1
} analog_channel_type;

typedef enum
{
	DFT = 0,
	HCDFT = 1,
	DST = 2,
	DCT = 3
} phasor_filter_type;

typedef struct
{
	bool is_enabled;
	analog_channel_type type;
	phasor_filter_type filter;
	float adc_to_sec_ratio;
	float itr_ratio;
} analog_channel_config;

typedef struct
{
	analog_channel_config channel_0a;
	analog_channel_config channel_1a;
	analog_channel_config channel_2a;
	analog_channel_config channel_3a;
	analog_channel_config channel_4a;
	analog_channel_config channel_5a;
	analog_channel_config channel_6a;
	analog_channel_config channel_7a;
	analog_channel_config channel_0b;
	analog_channel_config channel_1b;
	analog_channel_config channel_2b;
	analog_channel_config channel_3b;
	analog_channel_config channel_4b;
	analog_channel_config channel_5b;
	analog_channel_config channel_6b;
	analog_channel_config channel_7b;
} analog_config;

typedef struct
{
	bool is_enabled;
	analog_channel phase_a_channel;
	analog_channel phase_b_channel;
	analog_channel phase_c_channel;
	float pick_up;
} ansi50_config;

typedef struct
{
	bool is_enabled;
	analog_channel phase_a_channel;
	analog_channel phase_b_channel;
	analog_channel phase_c_channel;
	float pick_up;
	float time_dial;
	float a;
	float b;
	float c;
} ansi51_config;

typedef struct
{
	analog_config analog;
	ansi50_config ansi50[BAY_COUNT];
	ansi51_config ansi51[BAY_COUNT];
} general_config;

extern bool config_is_set;
void config_set(general_config* configs);
void config_get(general_config* out_configs);
void config_restore(void);