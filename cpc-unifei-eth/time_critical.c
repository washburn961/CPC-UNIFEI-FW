#include "time_critical.h"
#include "cmsis_os.h"
#include "main.h"
#include "spi.h"
#include "ads8686s.h"

#define ADC_CHANNEL_COUNT 16
#define SAMPLING_RATE_CONTROL_FLAG 0x0001
#define ADC_BUSY_FLAG 0x0002

osThreadId_t timeCriticalTaskHandle;
const osThreadAttr_t timeCriticalTask_attributes = {
	.name = "timeCriticalTask",
	.stack_size = 512 * 8,
	.priority = (osPriority_t) osPriorityRealtime,
};

struct ads8686s_device ads8686s;
struct ads8686s_conversion_result ads8686s_conversion_results[ADC_CHANNEL_COUNT / 2];

void time_critical_task(void *argument);
void adc_init(void);

void time_critical_init()
{
	timeCriticalTaskHandle = osThreadNew(time_critical_task, NULL, &timeCriticalTask_attributes);
}

void time_critical_task(void *argument)
{
	adc_init();
	
	// Start the first conversion before entering the loop
	GPIOD->BSRR = GPIO_PIN_12;
	GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
	
	while(1)
	{
		osThreadFlagsWait(ADC_BUSY_FLAG, osFlagsWaitAny, osWaitForever); // Waits for the busy pin to go low
		
		ads8686s_read_channels(&ads8686s, ads8686s_conversion_results); // Read conversion results
		
		// Pulse GPIO to start the next conversion
		GPIOD->BSRR = GPIO_PIN_12;
		GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
		
		osThreadFlagsWait(SAMPLING_RATE_CONTROL_FLAG, osFlagsWaitAny, osWaitForever); // Waits for the sampling rate period to end
	}
}

void adc_init(void)
{
	struct ads8686s_init_param ads8686s_init_param = {
		.osr = ADS8686S_OSR_128
	};
	
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
}

void time_critical_adc_timing_flag_set(void)
{
	osThreadFlagsSet(timeCriticalTaskHandle, SAMPLING_RATE_CONTROL_FLAG); // Signal to start conversion
}

void time_critical_adc_busy_flag_set(void)
{
	osThreadFlagsSet(timeCriticalTaskHandle, ADC_BUSY_FLAG); // Signal conversion complete
}

uint32_t time_critical_can_set_adc_busy_flag(uint16_t GPIO_Pin)
{
	return GPIO_Pin == ADC_BUSY_Pin && ads8686s.init_ok == 1 && (GPIOD->IDR & GPIO_PIN_11) == 0;
}