#pragma once

#include "stdint.h"
#include "stdbool.h"

#define CHANNEL_COUNT 16
#define SAMPLE_COUNT 16
#define BAY_COUNT 5
#define CONFIG_MAGIC_NUMBER ((uint32_t)0xDEADBEEF)

// Analog channel definitions, enforcing uint8_t type
#define CHANNEL_0A  ((uint8_t)0)
#define CHANNEL_0B  ((uint8_t)1)
#define CHANNEL_1A  ((uint8_t)2)
#define CHANNEL_1B  ((uint8_t)3)
#define CHANNEL_2A  ((uint8_t)4)
#define CHANNEL_2B  ((uint8_t)5)
#define CHANNEL_3A  ((uint8_t)6)
#define CHANNEL_3B  ((uint8_t)7)
#define CHANNEL_4A  ((uint8_t)8)
#define CHANNEL_4B  ((uint8_t)9)
#define CHANNEL_5A  ((uint8_t)10)
#define CHANNEL_5B  ((uint8_t)11)
#define CHANNEL_6A  ((uint8_t)12)
#define CHANNEL_6B  ((uint8_t)13)
#define CHANNEL_7A  ((uint8_t)14)
#define CHANNEL_7B  ((uint8_t)15)

// Analog channel type definitions, enforcing uint8_t type
#define VOLTAGE     ((uint8_t)0)
#define CURRENT     ((uint8_t)1)

// Phasor filter type definitions, enforcing uint8_t type
#define DFT         ((uint8_t)0)
#define HCDFT       ((uint8_t)1)
#define DST         ((uint8_t)2)
#define DCT         ((uint8_t)3)

typedef struct
{
	bool is_enabled;
	uint8_t type;
	uint8_t filter;
	float adc_to_sec_ratio;
	float itr_ratio;
} __attribute__((packed)) analog_channel_config;

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
} __attribute__((packed)) analog_config;

typedef struct
{
	bool is_enabled;
	uint8_t phase_a_channel;
	uint8_t phase_b_channel;
	uint8_t phase_c_channel;
	float pick_up;
} __attribute__((packed)) ansi50_config;

typedef struct
{
	bool is_enabled;
	uint8_t phase_a_channel;
	uint8_t phase_b_channel;
	uint8_t phase_c_channel;
	float pick_up;
	float time_dial;
	float a;
	float b;
	float c;
} __attribute__((packed)) ansi51_config;

typedef struct
{
	uint32_t magic_number;
	uint8_t version;
	uint32_t date;
	uint8_t reserved[4];
} __attribute__((packed)) general_config_header;

typedef struct
{
	general_config_header header;
	analog_config analog;
	ansi50_config ansi50[BAY_COUNT];
	ansi51_config ansi51[BAY_COUNT];
} __attribute__((packed)) general_config;

extern bool config_is_set;
void config_set(general_config* config);
void config_get(general_config* out_config);
void config_restore(void);