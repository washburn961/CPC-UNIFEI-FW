#define SAMPLING_RATE_CONTROL_FLAG 0x0001
#define ADC_BUSY_FLAG 0x0002
#define CHANNEL_COUNT 16
#define SAMPLE_COUNT 16

#include "application.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "spi.h"
#include "gpio.h"
#include "tim.h"
#include "cmsis_os.h"
#include "ads8686s.h"
#include "udp_server.h"
#include "gpio.h"
#include "fourier_transform.h"
#include "ring_buffer.h"
#include "goose_server.h"

void AnalogTask(void *argument);
void BlinkTask(void *argument);
void serialize_voltages(struct ads8686s_conversion_voltage *voltages, uint8_t* out_serialized, uint8_t size);

osSemaphoreId_t analogTaskMainSemaphore;
osSemaphoreId_t analogTaskConversionSemaphore;
osThreadId_t blinkTaskHandle;
osThreadId_t analogTaskHandle;
const osThreadAttr_t analogTask_attributes = {
	.name = "analogTask",
	.stack_size = 512 * 8,
	.priority = (osPriority_t) osPriorityRealtime,
};
const osThreadAttr_t blinkTask_attributes = {
	.name = "blinkTask",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};
struct ads8686s_device ads8686s;
struct ads8686s_init_param ads8686s_init_param = {
	.osr = ADS8686S_OSR_128
};
struct ads8686s_conversion_result conversion_buffer[CHANNEL_COUNT / 2];
struct ads8686s_conversion_voltage voltage_buffer[CHANNEL_COUNT / 2] = { 0 };
uint8_t serialized_voltages_tmp[CHANNEL_COUNT * sizeof(float)] = { 0 };
//ring_buffer ring_buffer_arr;
//float ring_buffer_content_arr[SAMPLE_COUNT];
ring_buffer ring_buffer_arr[CHANNEL_COUNT];
fourier_transform fourier_transform_arr[CHANNEL_COUNT];
float ring_buffer_content_arr[CHANNEL_COUNT][SAMPLE_COUNT] = { 0 };
float real_arr[CHANNEL_COUNT][SAMPLE_COUNT] = { 0 };
float imag_arr[CHANNEL_COUNT][SAMPLE_COUNT] = { 0 };
float sin_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };
float cos_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };
float ring_buffer_tmp[SAMPLE_COUNT] = { 0 };

uint32_t application_init(void)
{
//	ring_buffer_arr.content = ring_buffer_content_arr;
//	ring_buffer_arr.size = SAMPLE_COUNT;
//	ring_buffer_init(&ring_buffer_arr);
	for (uint8_t i = 0; i < CHANNEL_COUNT; i++)
	{
		ring_buffer_arr[i].content = ring_buffer_content_arr[i];
		ring_buffer_arr[i].size = SAMPLE_COUNT;
		ring_buffer_init(&ring_buffer_arr[i]);
		
		fourier_transform_arr[i].real = real_arr[i];
		fourier_transform_arr[i].imag = imag_arr[i];
		fourier_transform_arr[i].sin = sin_coef;
		fourier_transform_arr[i].cos = cos_coef;
		fourier_transform_arr[i].size = SAMPLE_COUNT;
		dft_init(&fourier_transform_arr[i]);
	}
	
	analogTaskHandle = osThreadNew(AnalogTask, NULL, &analogTask_attributes);
	blinkTaskHandle = osThreadNew(BlinkTask, NULL, &blinkTask_attributes);
	
	analogTaskMainSemaphore = osSemaphoreNew(1, 0, NULL);
	analogTaskConversionSemaphore = osSemaphoreNew(1, 0, NULL);
	
	return 0;
}

void AnalogTask(void *argument)
{
	uint16_t counter = 0;
	
	if (!ads8686s_init(&ads8686s, &ads8686s_init_param))
	{
		HAL_GPIO_WritePin(USER_LED2_GPIO_Port, USER_LED2_Pin, GPIO_PIN_SET);
		osDelay(100);
		HAL_GPIO_WritePin(USER_LED2_GPIO_Port, USER_LED2_Pin, GPIO_PIN_RESET);
		
		struct ads8686s_sequencer_layer layers[8] = { 0 };
		
		layers[0].ch_a = ADS8686S_VA0;
		layers[0].ch_b = ADS8686S_VB0;
		layers[1].ch_a = ADS8686S_VA1;
		layers[1].ch_b = ADS8686S_VB1;
		layers[2].ch_a = ADS8686S_VA2;
		layers[2].ch_b = ADS8686S_VB2;
		layers[3].ch_a = ADS8686S_VA3;
		layers[3].ch_b = ADS8686S_VB3;
		layers[4].ch_a = ADS8686S_VA4;
		layers[4].ch_b = ADS8686S_VB4;
		layers[5].ch_a = ADS8686S_VA5;
		layers[5].ch_b = ADS8686S_VB5;
		layers[6].ch_a = ADS8686S_VA6;
		layers[6].ch_b = ADS8686S_VB6;
		layers[7].ch_a = ADS8686S_VA7;
		layers[7].ch_b = ADS8686S_VB7;
	
		ads8686s_setup_sequencer(&ads8686s, layers, 8, 1);
		
		SPI1_HighFrequency_Init();
	}
	else
	{
		HAL_GPIO_WritePin(USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_SET);
	}
	
	HAL_TIM_Base_Start_IT(&htim2);
	
	// Start the first conversion before entering the loop
	GPIOD->BSRR = GPIO_PIN_12;
	GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
	
	while (true)
	{
		// Wait for the conversion complete signal (set by GPIO ISR)
		osThreadFlagsWait(ADC_BUSY_FLAG, osFlagsWaitAny, osWaitForever);
		
		// Read conversion results
		ads8686s_read_channels(&ads8686s, conversion_buffer);
		
		// Pulse GPIO to start the next conversion
		GPIOD->BSRR = GPIO_PIN_12;
		GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
		
		// Process the data
		for (uint8_t i = 0; i < CHANNEL_COUNT / 2; i++)
		{
			voltage_buffer[i].channel_a = (int16_t)conversion_buffer[i].channel_a * ads8686s.lsb;
			voltage_buffer[i].channel_b = (int16_t)conversion_buffer[i].channel_b * ads8686s.lsb;
			
			ring_buffer_write(&ring_buffer_arr[i * 2], voltage_buffer[i].channel_a);
			ring_buffer_write(&ring_buffer_arr[i * 2 + 1], voltage_buffer[i].channel_b);
			
			ring_buffer_read_many(&ring_buffer_arr[i * 2], ring_buffer_tmp, SAMPLE_COUNT);
			dft_step(&fourier_transform_arr[i * 2], ring_buffer_tmp);
			
			ring_buffer_read_many(&ring_buffer_arr[i * 2 + 1], ring_buffer_tmp, SAMPLE_COUNT);
			dft_step(&fourier_transform_arr[i * 2 + 1], ring_buffer_tmp);
		}
		
//		serialize_voltages(voltage_buffer, serialized_voltages_tmp, CHANNEL_COUNT / 2);
//		udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, serialized_voltages_tmp, sizeof(serialized_voltages_tmp));
		
		// Wait for the timer interrupt to control the sampling rate
		osThreadFlagsWait(SAMPLING_RATE_CONTROL_FLAG, osFlagsWaitAny, osWaitForever);
	}
}

void BlinkTask(void *argument)
{
	char keepAliveMessage[] = "CPC UNIFEI IS STILL ALIVE";
	uint8_t pin_states[8] = { 0 }; // Array to hold the state of all pins
	uint8_t counter = 0;

	while (true)
	{
		// Toggle the LED to show the system is alive
		HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
		osDelay(500);

		// Read the state of each IN[number]_[letter] pin and store it in the pin_states array
		pin_states[0] = (HAL_GPIO_ReadPin(IN1_A_GPIO_Port, IN1_A_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		pin_states[1] = (HAL_GPIO_ReadPin(IN2_A_GPIO_Port, IN2_A_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		pin_states[2] = (HAL_GPIO_ReadPin(IN1_B_GPIO_Port, IN1_B_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		pin_states[3] = (HAL_GPIO_ReadPin(IN2_B_GPIO_Port, IN2_B_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		pin_states[4] = (HAL_GPIO_ReadPin(IN1_C_GPIO_Port, IN1_C_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		pin_states[5] = (HAL_GPIO_ReadPin(IN2_C_GPIO_Port, IN2_C_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		pin_states[6] = (HAL_GPIO_ReadPin(IN1_D_GPIO_Port, IN1_D_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		pin_states[7] = (HAL_GPIO_ReadPin(IN2_D_GPIO_Port, IN2_D_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;

		// Serialize the pin states and send them via UDP
		udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, pin_states, sizeof(pin_states));

		if (++counter >= 10)
		{
			counter = 0;
			udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, keepAliveMessage, sizeof(keepAliveMessage));
		}

		// Send a GOOSE test message
		send_goose_test();
	}
}


// ISR or other function where the analog conversion is complete
void application_analog_semaphore_release(void)
{
	osThreadFlagsSet(analogTaskHandle, SAMPLING_RATE_CONTROL_FLAG); // Signal to start conversion
}

void application_analog_busy_semaphore_release(void)
{
	osThreadFlagsSet(analogTaskHandle, ADC_BUSY_FLAG); // Signal conversion complete
}

uint32_t application_analog_busy_semaphore_can_release(uint16_t GPIO_Pin)
{
	return GPIO_Pin == ADC_BUSY_Pin && ads8686s.init_ok == 1 && (GPIOD->IDR & GPIO_PIN_11) == 0;
}

void serialize_voltages(struct ads8686s_conversion_voltage *voltages, uint8_t* out_serialized, uint8_t size)
{
	// Calculate the size of each struct in bytes
	size_t struct_size = sizeof(struct ads8686s_conversion_voltage);

	// Loop through the array of voltages
	for (uint8_t i = 0; i < size; i++) {
		// Serialize the current struct to the byte array
		memcpy(out_serialized + i * struct_size, &voltages[i], struct_size);
	}
}