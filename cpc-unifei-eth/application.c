#include "application.h"
#include <stdio.h>
#include "debug.h"
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
#include "ring_buffer.h"
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip.h"
#include "is25lp064a.h"

#define SAMPLING_RATE_CONTROL_FLAG 0x0001
#define ADC_BUSY_FLAG 0x0002
#define GOOSE_TASK_FLAG 0x0003
#define CHANNEL_COUNT 16
#define SAMPLE_COUNT 16

//void AnalogTask(void *argument);
void BlinkTask(void *argument);
//void serialize_voltages(struct ads8686s_conversion_voltage *voltages, uint8_t* out_serialized, uint8_t size);
//void link_output(uint8_t* byte_stream, size_t length);
//void send_float_over_udp(float value);


osThreadId_t blinkTaskHandle;
//osThreadId_t analogTaskHandle;
//const osThreadAttr_t analogTask_attributes = {
//	.name = "analogTask",
//	.stack_size = 512 * 8,
//	.priority = (osPriority_t) osPriorityRealtime,
//};
const osThreadAttr_t blinkTask_attributes = {
	.name = "blinkTask",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};
//struct ads8686s_device ads8686s;
//struct ads8686s_init_param ads8686s_init_param = {
//	.osr = ADS8686S_OSR_128
//};
//struct ads8686s_conversion_result conversion_buffer[CHANNEL_COUNT / 2];
//struct ads8686s_conversion_voltage voltage_buffer[CHANNEL_COUNT / 2] = { 0 };
//uint8_t serialized_voltages_tmp[CHANNEL_COUNT * sizeof(float)] = { 0 };
//ring_buffer ring_buffer_arr[CHANNEL_COUNT];
//fourier_transform fourier_transform_arr[CHANNEL_COUNT];
//float ring_buffer_content_arr[CHANNEL_COUNT][SAMPLE_COUNT] = { 0 };
//float real_arr[CHANNEL_COUNT][SAMPLE_COUNT] = { 0 };
//float imag_arr[CHANNEL_COUNT][SAMPLE_COUNT] = { 0 };
//float sin_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };
//float cos_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };
//float ring_buffer_tmp[SAMPLE_COUNT] = { 0 };

uint32_t application_init(void)
{
//	ring_buffer_arr.content = ring_buffer_content_arr;
//	ring_buffer_arr.size = SAMPLE_COUNT;
//	ring_buffer_init(&ring_buffer_arr);
//	for (uint8_t i = 0; i < CHANNEL_COUNT; i++)
//	{
//		ring_buffer_arr[i].content = ring_buffer_content_arr[i];
//		ring_buffer_arr[i].size = SAMPLE_COUNT;
//		ring_buffer_init(&ring_buffer_arr[i]);
//		
//		fourier_transform_arr[i].real = real_arr[i];
//		fourier_transform_arr[i].imag = imag_arr[i];
//		fourier_transform_arr[i].sin = sin_coef;
//		fourier_transform_arr[i].cos = cos_coef;
//		fourier_transform_arr[i].size = SAMPLE_COUNT;
//		dft_init(&fourier_transform_arr[i]);
//	}
//	
//	analogTaskHandle = osThreadNew(AnalogTask, NULL, &analogTask_attributes);
	blinkTaskHandle = osThreadNew(BlinkTask, NULL, &blinkTask_attributes);
	
//	HAL_TIM_Base_Start_IT(&htim2);
//	HAL_TIM_Base_Start_IT(&htim3);
//	
//	is25lp064a_init();
	
	return 0;
}

//void AnalogTask(void *argument)
//{
//	osDelay(1000);
//	uint16_t counter = 0;
//	
//	if (!ads8686s_init(&ads8686s, &ads8686s_init_param))
//	{
//		DEBUG_INFO("%s", "ADS8686S INIT OK\r\n");
//		
//		struct ads8686s_sequencer_layer layers[8] = { 0 };
//		
//		layers[0].ch_a = ADS8686S_VA0;
//		layers[0].ch_b = ADS8686S_VB0;
//		layers[1].ch_a = ADS8686S_VA1;
//		layers[1].ch_b = ADS8686S_VB1;
//		layers[2].ch_a = ADS8686S_VA2;
//		layers[2].ch_b = ADS8686S_VB2;
//		layers[3].ch_a = ADS8686S_VA3;
//		layers[3].ch_b = ADS8686S_VB3;
//		layers[4].ch_a = ADS8686S_VA4;
//		layers[4].ch_b = ADS8686S_VB4;
//		layers[5].ch_a = ADS8686S_VA5;
//		layers[5].ch_b = ADS8686S_VB5;
//		layers[6].ch_a = ADS8686S_VA6;
//		layers[6].ch_b = ADS8686S_VB6;
//		layers[7].ch_a = ADS8686S_VA7;
//		layers[7].ch_b = ADS8686S_VB7;
//	
//		ads8686s_setup_sequencer(&ads8686s, layers, 8, 1);
//		
//		SPI1_HighFrequency_Init();
//	}
//	else
//	{
//		DEBUG_ERROR("%s", "ADS8686S INIT FAILED\r\n");
//	}
//	
//	// Start the first conversion before entering the loop
//	GPIOD->BSRR = GPIO_PIN_12;
//	GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
//	
//	while (true)
//	{
//		// Wait for the conversion complete signal (set by GPIO ISR)
//		osThreadFlagsWait(ADC_BUSY_FLAG, osFlagsWaitAny, osWaitForever);
//		
//		// Read conversion results
//		ads8686s_read_channels(&ads8686s, conversion_buffer);
//		
//		// Pulse GPIO to start the next conversion
//		GPIOD->BSRR = GPIO_PIN_12;
//		GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
//		
//		// Process the data
//		for (uint8_t i = 0; i < CHANNEL_COUNT / 2; i++)
//		{
//			voltage_buffer[i].channel_a = (int16_t)conversion_buffer[i].channel_a * ads8686s.lsb;
//			voltage_buffer[i].channel_b = (int16_t)conversion_buffer[i].channel_b * ads8686s.lsb;
//			
//			ring_buffer_write(&ring_buffer_arr[i * 2], voltage_buffer[i].channel_a);
//			ring_buffer_write(&ring_buffer_arr[i * 2 + 1], voltage_buffer[i].channel_b);
//			
//			ring_buffer_read_many(&ring_buffer_arr[i * 2], ring_buffer_tmp, SAMPLE_COUNT);
//			dft_step(&fourier_transform_arr[i * 2], ring_buffer_tmp);
//			
//			ring_buffer_read_many(&ring_buffer_arr[i * 2 + 1], ring_buffer_tmp, SAMPLE_COUNT);
//			dft_step(&fourier_transform_arr[i * 2 + 1], ring_buffer_tmp);
//		}
//		
//		osThreadFlagsWait(SAMPLING_RATE_CONTROL_FLAG, osFlagsWaitAny, osWaitForever);
//	}
//}

void BlinkTask(void *argument)
{
	while (true)
	{
		HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
		osDelay(500);
	}
}

// ISR or other function where the analog conversion is complete
//void application_adc_timing_flag_set(void)
//{
//	osThreadFlagsSet(analogTaskHandle, SAMPLING_RATE_CONTROL_FLAG); // Signal to start conversion
//}
//
//void application_adc_busy_flag_set(void)
//{
//	osThreadFlagsSet(analogTaskHandle, ADC_BUSY_FLAG); // Signal conversion complete
//}
//
//uint32_t application_analog_busy_semaphore_can_release(uint16_t GPIO_Pin)
//{
//	return GPIO_Pin == ADC_BUSY_Pin && ads8686s.init_ok == 1 && (GPIOD->IDR & GPIO_PIN_11) == 0;
//}

//void link_output(uint8_t* byte_stream, size_t length)
//{
//	struct pbuf *p;
//
//	LOCK_TCPIP_CORE();
//
//	// Allocate a pbuf for the Ethernet frame
//	p = pbuf_alloc(PBUF_RAW, length, PBUF_RAM);
//	if (p == NULL) {
//		// Handle allocation failure
//		UNLOCK_TCPIP_CORE();
//		return;
//	}
//
//	// Copy the Ethernet frame into the pbuf
//	memcpy(p->payload, byte_stream, length);
//
//	// Send the pbuf using the linkoutput function
//	if (gnetif.linkoutput(&gnetif, p) != ERR_OK) {
//		// Handle send error
//	}
//
//	// Free the pbuf
//	pbuf_free(p);
//    
//	UNLOCK_TCPIP_CORE();
//}