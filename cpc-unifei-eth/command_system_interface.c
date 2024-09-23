#include "command_system_interface.h"
#include "main.h"
#include "udp_server.h"
#include "ads8686s.h"
#include "stdio.h"

// Example function implementations for GPIO
void write_OUT1_B(int value) {
	HAL_GPIO_WritePin(OUT1_B_OUT_GPIO_Port, OUT1_B_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int read_OUT1_B(void) {
	uint8_t state = HAL_GPIO_ReadPin(OUT1_B_OUT_GPIO_Port, OUT1_B_OUT_Pin);
	
	return 0;
}

void write_OUT2_B(int value) {
	HAL_GPIO_WritePin(OUT2_B_OUT_GPIO_Port, OUT2_B_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int read_OUT2_B(void) {
	uint8_t state = HAL_GPIO_ReadPin(OUT2_B_OUT_GPIO_Port, OUT2_B_OUT_Pin);
	
	return 0;
}

void write_OUT3_B(int value) {
	HAL_GPIO_WritePin(OUT3_B_OUT_GPIO_Port, OUT3_B_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int read_OUT3_B(void) {
	uint8_t state = HAL_GPIO_ReadPin(OUT3_B_OUT_GPIO_Port, OUT3_B_OUT_Pin);
	
	return 0;
}

void write_OUT4_B(int value) {
	HAL_GPIO_WritePin(OUT4_B_OUT_GPIO_Port, OUT4_B_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int read_OUT4_B(void) {
	uint8_t state = HAL_GPIO_ReadPin(OUT4_B_OUT_GPIO_Port, OUT4_B_OUT_Pin);
	
	return 0;
}

void write_OUT1_A(int value) {
	HAL_GPIO_WritePin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int read_OUT1_A(void) {
	uint8_t state = HAL_GPIO_ReadPin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin);
	
	return 0;
}

void write_OUT2_A(int value) {
	HAL_GPIO_WritePin(OUT2_A_OUT_GPIO_Port, OUT2_A_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int read_OUT2_A(void) {
	uint8_t state = HAL_GPIO_ReadPin(OUT2_A_OUT_GPIO_Port, OUT2_A_OUT_Pin);
	
	return 0;
}

void write_OUT3_A(int value) {
	HAL_GPIO_WritePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int read_OUT3_A(void) {
	uint8_t state = HAL_GPIO_ReadPin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin);
	
	return 0;
}

void write_OUT4_A(int value) {
	HAL_GPIO_WritePin(OUT4_A_OUT_GPIO_Port, OUT4_A_OUT_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

int read_OUT4_A(void) {
	uint8_t state = HAL_GPIO_ReadPin(OUT4_A_OUT_GPIO_Port, OUT4_A_OUT_Pin);
	
	return 0;
}

uint8_t adc_register_value = 0x2;

void write_ADC_SET(int value) {
	adc_register_value = (uint8_t)value;
}

int read_ADC_SET(void) {
	char message[50];
	int message_length = snprintf(message, sizeof(message), "ADC_SET=%u", adc_register_value);
	udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, message, sizeof(message));
	
	return 0;
}

void write_ADC(int value) {
	ads8686s_write(&dev, adc_register_value, (uint16_t)value);
}

int read_ADC(void) {
	char message[50];
	uint16_t ret = 0;
	ads8686s_read(&dev, adc_register_value, &ret);
	int message_length = snprintf(message, sizeof(message), "ADC_REG=%u", ret);
	udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, message, sizeof(message));
	
	return 0;
}

// Array mapping virtual addresses to corresponding functions
VirtualAddress address_map[] = {
	{ "OUT1_B", write_OUT1_B, read_OUT1_B },
	{ "OUT2_B", write_OUT2_B, read_OUT2_B },
	{ "OUT3_B", write_OUT3_B, read_OUT3_B },
	{ "OUT4_B", write_OUT4_B, read_OUT4_B },
	{ "OUT1_A", write_OUT1_A, read_OUT1_A },
	{ "OUT2_A", write_OUT2_A, read_OUT2_A },
	{ "OUT3_A", write_OUT3_A, read_OUT3_A },
	{ "OUT4_A", write_OUT4_A, read_OUT4_A },
	{ "ADC_SET", write_ADC_SET, read_ADC_SET },
	{ "ADC_REG", write_ADC, read_ADC },
	// You can add even more mappings here if needed
};

// Define the size of the address_map array
const unsigned int address_map_size = sizeof(address_map) / sizeof(VirtualAddress);