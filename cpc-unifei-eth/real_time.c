#include "real_time.h"
#include "config.h"
#include "signal_processing.h"
#include "ads8686s.h"
#include "cmsis_os.h"
#include "tim.h"
#include "spi.h"
#include "gpio.h"
#include "debug.h"

#define SAMPLING_RATE_CONTROL_FLAG 1
#define ADC_BUSY_FLAG 2
#define GOOSE_TASK_FLAG 3

struct ads8686s_device ads8686s;
struct ads8686s_conversion_result conversion_buffer[CHANNEL_COUNT / 2];
osMutexId_t real_time_mutex_handle;
const osMutexAttr_t real_time_mutex_attributes = {
	.name = "real_time_mutex"
};
osThreadId_t real_time_task_handle;
const osThreadAttr_t real_time_task_attributes = {
	.name = "real_time_task",
	.stack_size = 512 * 8,
	.priority = (osPriority_t) osPriorityRealtime,
};

void real_time_task(void *argument);
void adc_setup(void);
void execute_signal_processing(void);

void real_time_init(void)
{
	real_time_mutex_handle = osMutexNew(&real_time_mutex_attributes);
	real_time_task_handle = osThreadNew(real_time_task, NULL, &real_time_task_attributes);
	
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
}

void real_time_task(void *argument)
{
	adc_setup();
	
	// Pulse GPIO to start the first conversion
	GPIOD->BSRR = GPIO_PIN_12;
	GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
	
	while (1)
	{
//		real_time_take();
		
		// Wait for the conversion complete signal (set by GPIO ISR)
		osThreadFlagsWait(ADC_BUSY_FLAG, osFlagsWaitAny, osWaitForever);
		
		// Read conversion results
		ads8686s_read_channels(&ads8686s, conversion_buffer);
		
		// Do stuff here as needed
		execute_signal_processing();
		
		// Pulse GPIO to start the next conversion
		GPIOD->BSRR = GPIO_PIN_12;
		GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
		
		osThreadFlagsWait(SAMPLING_RATE_CONTROL_FLAG, osFlagsWaitAny, osWaitForever);
		
//		real_time_release();
	}
}

void adc_setup(void)
{
	struct ads8686s_init_param ads8686s_init_param = {
		.osr = ADS8686S_OSR_128
	};
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
	
	if (!ads8686s_init(&ads8686s, &ads8686s_init_param))
	{
		ads8686s_setup_sequencer(&ads8686s, layers, 8, 1);
		SPI1_HighFrequency_Init();
		DEBUG_INFO("%s", "ADS8686S INIT OK");
	}
	else
	{
		DEBUG_ERROR("%s", "ADS8686S INIT FAILED");
	}
}

void real_time_adc_timing_flag_set(void)
{
	osThreadFlagsSet(real_time_task_handle, SAMPLING_RATE_CONTROL_FLAG); // Signal to start conversion
}

void real_time_adc_busy_flag_set(void)
{
	osThreadFlagsSet(real_time_task_handle, ADC_BUSY_FLAG); // Signal conversion complete
}

uint32_t real_time_analog_busy_semaphore_can_release(uint16_t GPIO_Pin)
{
	return GPIO_Pin == ADC_BUSY_Pin && ads8686s.init_ok == 1 && (GPIOD->IDR & GPIO_PIN_11) == 0;
}

void execute_signal_processing(void)
{
	for (size_t i = 0; i < (CHANNEL_COUNT / 2); i++)
	{
		signal_processing_step((uint8_t)(2 * i), (conversion_buffer[i].channel_a * ads8686s.lsb));
		signal_processing_step((uint8_t)(2 * i + 1), (conversion_buffer[i].channel_b * ads8686s.lsb));
	}
}

void real_time_take(void)
{
	osMutexAcquire(real_time_mutex_handle, osWaitForever);
}

void real_time_release(void)
{
	osMutexRelease(real_time_mutex_handle);
}