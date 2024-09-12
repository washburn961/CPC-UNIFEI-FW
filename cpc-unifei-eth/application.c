#include "application.h"
#include "FreeRTOS.h"
#include "task.h"
#include "spi.h"
#include "gpio.h"
#include "tim.h"
#include "cmsis_os.h"
#include "ads8686s.h"
#include "udp_server.h"
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void AnalogTask(void *argument);
void BlinkTask(void *argument);

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

uint32_t application_init(void)
{
	analogTaskHandle = osThreadNew(AnalogTask, NULL, &analogTask_attributes);
	blinkTaskHandle = osThreadNew(BlinkTask, NULL, &blinkTask_attributes);
	
	analogTaskMainSemaphore = osSemaphoreNew(1, 0, NULL);
	analogTaskConversionSemaphore = osSemaphoreNew(1, 0, NULL);
	
	return 0;
}

struct ads8686s_conversion_result conversion_buffer[8];

static void SPI_HighFrequency_Init(void)
{
	HAL_SPI_Abort(&hspi1);
	HAL_SPI_DeInit(&hspi1);
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 0x0;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
	hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
	hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
	hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
	hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_01CYCLE;
	hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
	hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
	hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
	hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		Error_Handler();
	}
}

struct ads8686s_conversion_voltage voltages[8] = { 0 };

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
		
		SPI_HighFrequency_Init();
	}
	else
	{
		HAL_GPIO_WritePin(USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_SET);
	}
	
//	ring_buffer_init(&buffer);
	
	HAL_TIM_Base_Start_IT(&htim2);
	
	// Start the first conversion before entering the loop
	GPIOD->BSRR = GPIO_PIN_12;
	GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
	
	while (true)
	{
		// Wait for the conversion complete signal (set by GPIO ISR)
		osThreadFlagsWait(0x0002, osFlagsWaitAny, osWaitForever);
		
		// Read conversion results
		ads8686s_read_channels(&ads8686s, conversion_buffer);
		
		// Pulse GPIO to start the next conversion
		GPIOD->BSRR = GPIO_PIN_12;
		GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
		
		// Process the data
		for (uint8_t i = 0; i < 8; i++)
		{
			voltages[i].channel_a = (int16_t)conversion_buffer[i].channel_a * ads8686s.lsb;
			voltages[i].channel_b = (int16_t)conversion_buffer[i].channel_b * ads8686s.lsb;
		}
		
		// Wait for the timer interrupt to control the sampling rate
		osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);
	}
}

void BlinkTask(void *argument)
{
	char keepAliveMessage[] = "CPC UNIFEI IS STILL ALIVE";
	uint8_t counter = 0;
	
	while (true)
	{
		HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
		osDelay(500);
		if (++counter >= 10)
		{
			counter = 0;
			udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, keepAliveMessage, sizeof(keepAliveMessage));
		}
	}
}

// ISR or other function where the analog conversion is complete
void application_analog_semaphore_release(void)
{
	osThreadFlagsSet(analogTaskHandle, 0x0001); // Signal to start conversion
}

void application_analog_busy_semaphore_release(void)
{
	osThreadFlagsSet(analogTaskHandle, 0x0002); // Signal conversion complete
}

uint32_t application_analog_busy_semaphore_can_release(uint16_t GPIO_Pin)
{
	return GPIO_Pin == ADC_BUSY_Pin && ads8686s.init_ok == 1 && (GPIOD->IDR & GPIO_PIN_11) == 0;
}