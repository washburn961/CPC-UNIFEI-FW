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
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip.h"
#include "goose_frame.h"
#include "goose_publisher.h"
#include "ANSI51.h"
#include "ANSI50.h"
#include "cpc_complex.h"

#define SAMPLING_RATE_CONTROL_FLAG 0x0001
#define ADC_BUSY_FLAG 0x0002
#define GOOSE_TASK_FLAG 0x0003
#define CHANNEL_COUNT 16
#define SAMPLE_COUNT 16
#define VTOC_RATIO 10.0f
#define VTOC(v) ((v) * VTOC_RATIO)


void AnalogTask(void *argument);
void BlinkTask(void *argument);
void GooseTask(void *argument);
void OUT1Task(void *argument);
void OUT3Task(void *argument);
void InputMonitoringTask(void *argument);
void serialize_voltages(struct ads8686s_conversion_voltage *voltages, uint8_t* out_serialized, uint8_t size);
void link_output(uint8_t* byte_stream, size_t length);
void parametrize_overcurrent_protection(void);
void relay_outputs_selftest(void);
void send_float_over_udp(float value);

ANSI51 ansi51_element;
uint32_t ansi51_status;
ANSI50 ansi50_element;
uint32_t ansi50_status;
uint8_t input_status[9] = { 0 };
goose_message_params input_mon_goose_params;
osThreadId_t blinkTaskHandle;
osThreadId_t analogTaskHandle;
osThreadId_t out1TaskHandle;
osThreadId_t out3TaskHandle;
osThreadId_t gooseTaskHandle;
osThreadId_t inputMonitoringTaskHandle;
const osThreadAttr_t analogTask_attributes = {
	.name = "analogTask",
	.stack_size = 512 * 8,
	.priority = (osPriority_t) osPriorityRealtime,
};
const osThreadAttr_t out1Task_attributes = {
	.name = "out1Task",
	.stack_size = 512,
	.priority = (osPriority_t) osPriorityHigh,
};
const osThreadAttr_t out3Task_attributes = {
	.name = "out3Task",
	.stack_size = 512,
	.priority = (osPriority_t) osPriorityHigh,
};
const osThreadAttr_t gooseTask_attributes = {
	.name = "gooseTask",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityHigh,
};
const osThreadAttr_t blinkTask_attributes = {
	.name = "blinkTask",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};
const osThreadAttr_t inputMonitoringTask_attributes = {
	.name = "inputMonitoringTask",
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
	inputMonitoringTaskHandle = osThreadNew(InputMonitoringTask, NULL, &inputMonitoringTask_attributes);
	gooseTaskHandle = osThreadNew(GooseTask, NULL, &gooseTask_attributes);
	out1TaskHandle = osThreadNew(OUT1Task, NULL, &out1Task_attributes);
	out3TaskHandle = osThreadNew(OUT3Task, NULL, &out3Task_attributes);
	
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
	
	return 0;
}

void AnalogTask(void *argument)
{
//	relay_outputs_selftest();
	parametrize_overcurrent_protection();
	
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
	
	// Start the first conversion before entering the loop
	GPIOD->BSRR = GPIO_PIN_12;
	GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
	
	complex_t complex_tmp[3];
	float temp;
	
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
		
		complex_tmp[0].real = VTOC(real_arr[0][1]);
		complex_tmp[0].imag = VTOC(imag_arr[0][1]);
		ansi51_element.current[0] = complex_tmp[0];
		ansi50_element.current[0] = complex_tmp[0];
		
		complex_tmp[1].real = VTOC(real_arr[2][1]);
		complex_tmp[1].imag = VTOC(imag_arr[2][1]);
		ansi51_element.current[1] = complex_tmp[1];
		ansi50_element.current[1] = complex_tmp[1];
		
		complex_tmp[2].real = VTOC(real_arr[4][1]);
		complex_tmp[2].imag = VTOC(imag_arr[4][1]);
		ansi51_element.current[2] = complex_tmp[2];
		ansi50_element.current[2] = complex_tmp[2];
		
		ANSI51_Step(&ansi51_element);
		ANSI50_Step(&ansi50_element);
		
		dft_get_magnitude(&fourier_transform_arr[2], &temp, 1);
		send_float_over_udp(temp);
		
		
//		if (ansi51_element.is_tripped[0] || ansi51_element.is_tripped[1] || ansi51_element.is_tripped[2])
//		{
//			HAL_GPIO_WritePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin, GPIO_PIN_SET);
//		}
//		
//		if (ansi50_element.is_tripped[0] || ansi50_element.is_tripped[1] || ansi50_element.is_tripped[2])
//		{
//			HAL_GPIO_WritePin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin, GPIO_PIN_SET);
//		}
		
		ansi51_status = (ansi51_element.is_tripped[0] || ansi51_element.is_tripped[1] || ansi51_element.is_tripped[2]) ? 0x1 : 0x0;
		ansi50_status = (ansi50_element.is_tripped[0] || ansi50_element.is_tripped[1] || ansi50_element.is_tripped[2]) ? 0x1 : 0x0;
		
		// Wait for the timer interrupt to control the sampling rate
		osThreadFlagsWait(SAMPLING_RATE_CONTROL_FLAG, osFlagsWaitAny, osWaitForever);
	}
}

void OUT1Task(void *argument)
{
	GPIO_PinState prev_pin_state = GPIO_PIN_RESET;
	GPIO_PinState pin_state = GPIO_PIN_RESET;
	while (true)
	{
		pin_state = ansi50_status ? GPIO_PIN_SET : GPIO_PIN_RESET;
		if (prev_pin_state != pin_state)
		{
			HAL_GPIO_WritePin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin, pin_state);
			
			if (prev_pin_state == GPIO_PIN_RESET)
			{
				osDelay(100);
			}
			
			prev_pin_state = pin_state;
		}
	}
}

void OUT3Task(void *argument)
{
	GPIO_PinState prev_pin_state = GPIO_PIN_RESET;
	GPIO_PinState pin_state = GPIO_PIN_RESET;
	while (true)
	{
		pin_state = ansi51_status ? GPIO_PIN_SET : GPIO_PIN_RESET;
		
		if (prev_pin_state != pin_state)
		{
			HAL_GPIO_WritePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin, pin_state);
			
			if (prev_pin_state == GPIO_PIN_RESET)
			{
				osDelay(100);
			}
			
			prev_pin_state = pin_state;
		}
	}
}

void BlinkTask(void *argument)
{
	char keepAliveMessage[] = "CPC UNIFEI IS STILL ALIVE";
	uint8_t counter = 0;

	while (true)
	{
		// Toggle the LED to show the system is alive
		HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
		osDelay(500);

		if (++counter >= 10)
		{
			counter = 0;
			udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, keepAliveMessage, sizeof(keepAliveMessage));
		}
	}
}

void GooseTask(void *argument)
{
	goose_publisher_init(&link_output);
	
	while (true)
	{
		osThreadFlagsWait(GOOSE_TASK_FLAG, osFlagsWaitAny, osWaitForever);
		goose_publisher_process();
	}
}

void InputMonitoringTask(void *argument)
{
	goose_handle* input_mon_goose_handle = goose_init(gnetif.hwaddr, destination, app_id);
	
	ber_set(&(input_mon_goose_handle->frame->pdu_list.gocbref), (uint8_t*)gocbRef, strlen(gocbRef));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.dataset), (uint8_t*)dataset, strlen(dataset));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.go_id), (uint8_t*)go_id, strlen(go_id));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.time_allowed_to_live), (uint8_t*)&time_allowed_to_live, sizeof(time_allowed_to_live));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.t), (uint8_t*)&t, sizeof(t));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.st_num), (uint8_t*)&st_num, sizeof(st_num));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.sq_num), (uint8_t*)&sq_num, sizeof(sq_num));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.simulation), (uint8_t*)&simulation, sizeof(simulation));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.conf_rev), (uint8_t*)&conf_rev, sizeof(conf_rev));
	ber_set(&(input_mon_goose_handle->frame->pdu_list.nds_com), (uint8_t*)&nds_com, sizeof(nds_com));
		
	for (size_t i = 0; i < 9; i++)
	{
		goose_all_data_entry_add(input_mon_goose_handle, 0x83, sizeof(input_status[i]), &(input_status[i]));
	}
	
	input_mon_goose_params.default_time_allowed_to_live = 1000;
	input_mon_goose_params.name = "inputMonitoring";
	input_mon_goose_params.handle = input_mon_goose_handle;
	
	goose_publisher_register(input_mon_goose_params);
	
	uint8_t different_data_flag = 0;
	uint8_t input_status_tmp[9] = { 0 };
	
	while (true)
	{
		input_status_tmp[0] = (HAL_GPIO_ReadPin(IN1_A_GPIO_Port, IN1_A_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		input_status_tmp[1] = (HAL_GPIO_ReadPin(IN2_A_GPIO_Port, IN2_A_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		input_status_tmp[2] = (HAL_GPIO_ReadPin(IN1_B_GPIO_Port, IN1_B_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		input_status_tmp[3] = (HAL_GPIO_ReadPin(IN2_B_GPIO_Port, IN2_B_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		input_status_tmp[4] = (HAL_GPIO_ReadPin(IN1_C_GPIO_Port, IN1_C_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		input_status_tmp[5] = (HAL_GPIO_ReadPin(IN2_C_GPIO_Port, IN2_C_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		input_status_tmp[6] = (HAL_GPIO_ReadPin(IN1_D_GPIO_Port, IN1_D_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		input_status_tmp[7] = (HAL_GPIO_ReadPin(IN2_D_GPIO_Port, IN2_D_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		input_status_tmp[8] = (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		
		for (size_t i = 0; i < 9; i++)
		{
			if (input_status_tmp[i] != input_status[i])
			{
				goose_all_data_entry_modify(input_mon_goose_handle, i, 0x83, sizeof(input_status_tmp[i]), &(input_status_tmp[i]));
				different_data_flag = 1;
			}
			
			input_status[i] = input_status_tmp[i];
		}
		
		if (different_data_flag)
		{
			goose_publisher_notify(input_mon_goose_params.name);
			different_data_flag = 0;
		}
	}
}

// ISR or other function where the analog conversion is complete
void application_adc_timing_flag_set(void)
{
	osThreadFlagsSet(analogTaskHandle, SAMPLING_RATE_CONTROL_FLAG); // Signal to start conversion
}

void application_adc_busy_flag_set(void)
{
	osThreadFlagsSet(analogTaskHandle, ADC_BUSY_FLAG); // Signal conversion complete
}

void application_goose_flag_set(void)
{
	osThreadFlagsSet(gooseTaskHandle, GOOSE_TASK_FLAG);
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

void link_output(uint8_t* byte_stream, size_t length)
{
	struct pbuf *p;

	LOCK_TCPIP_CORE();

	// Allocate a pbuf for the Ethernet frame
	p = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);
	if (p == NULL) {
		// Handle allocation failure
		UNLOCK_TCPIP_CORE();
		return;
	}

	// Copy the Ethernet frame into the pbuf
	memcpy(p->payload, byte_stream, length);

	// Send the pbuf using the linkoutput function
	if (gnetif.linkoutput(&gnetif, p) != ERR_OK) {
		// Handle send error
	}

	// Free the pbuf
	pbuf_free(p);
    
	UNLOCK_TCPIP_CORE();
}

void parametrize_overcurrent_protection()
{
	ANSI51_Init(&ansi51_element, 0.5f, 5.5f, 0.00104166666f, STANDARD_2, 1);
	ANSI50_Init(&ansi50_element, 17);
}

void relay_outputs_selftest(void)
{
	HAL_GPIO_TogglePin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT2_A_OUT_GPIO_Port, OUT2_A_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT4_A_OUT_GPIO_Port, OUT4_A_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT1_B_OUT_GPIO_Port, OUT1_B_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT2_B_OUT_GPIO_Port, OUT2_B_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT3_B_OUT_GPIO_Port, OUT3_B_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT4_B_OUT_GPIO_Port, OUT4_B_OUT_Pin);
	osDelay(500);
	
	HAL_GPIO_TogglePin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT2_A_OUT_GPIO_Port, OUT2_A_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT4_A_OUT_GPIO_Port, OUT4_A_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT1_B_OUT_GPIO_Port, OUT1_B_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT2_B_OUT_GPIO_Port, OUT2_B_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT3_B_OUT_GPIO_Port, OUT3_B_OUT_Pin);
	osDelay(200);
	HAL_GPIO_TogglePin(OUT4_B_OUT_GPIO_Port, OUT4_B_OUT_Pin);
}

void send_float_over_udp(float value)
{
	char buffer[32]; // Create a buffer to hold the string representation of the float

	// Convert the float to a string with 8 decimal places
	snprintf(buffer, sizeof(buffer), "%.8f", value);

	// Send the string over UDP using the default address and port
	udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, buffer, (uint16_t)strlen(buffer));
}