#include "real_time.h"
#include "config.h"
#include "signal_processing.h"
#include "ads8686s.h"
#include "cmsis_os.h"
#include "tim.h"
#include "spi.h"
#include "gpio.h"
#include "debug.h"
#include "ANSI87T.h"
#include "udp_server.h"

#define SAMPLING_RATE_CONTROL_FLAG 1
#define ADC_BUSY_FLAG 2
#define GOOSE_TASK_FLAG 3
#define MAX_STRING_SIZE 512  // Adjust size as needed

void generate_and_send_magnitude_string(void)
{
	char mag_string[MAX_STRING_SIZE];
	int offset = 0;

	// Collect only the channels from your task
	const uint8_t channels_to_process[] = {
		CHANNEL_0A,
		CHANNEL_1A,
		CHANNEL_2A, 
		CHANNEL_4A,
		CHANNEL_5A,
		CHANNEL_6A
	};

	for (size_t i = 0; i < sizeof(channels_to_process) / sizeof(channels_to_process[0]); i++)
	{
		float magnitude;
		uint8_t channel = channels_to_process[i];

		signal_processing_mag_get(channel, 1, &magnitude);

		// Append channel and magnitude to the string
		int written = snprintf(
		    mag_string + offset, 
			MAX_STRING_SIZE - offset, 
			"CH%d = %.2f\r\n", 
			channel, 
			magnitude);

		// Handle buffer overflow
		if (written < 0 || written >= (MAX_STRING_SIZE - offset))
		{
			// Stop processing if buffer overflows
			break;
		}

		offset += written;
	}

	// Send the string via UDP
	udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, mag_string, offset);
}

ANSI87T ansi87t;
general_config test_config = { 0 };
struct ads8686s_device ads8686s;
struct ads8686s_conversion_result conversion_buffer[CHANNEL_COUNT / 2];
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
	test_config.header.magic_number = CONFIG_MAGIC_NUMBER;
	test_config.header.uid = 0xdeadbeee;
	test_config.header.version = CONFIG_VERSION;
	
	test_config.analog.channel_0a.adc_to_sec_ratio = 1;
	test_config.analog.channel_0a.filter = DFT;
	test_config.analog.channel_0a.is_enabled = true;
	test_config.analog.channel_0a.itr_ratio = 1;
	test_config.analog.channel_0a.type = CURRENT;
	
	test_config.analog.channel_1a.adc_to_sec_ratio = 1;
	test_config.analog.channel_1a.filter = DFT;
	test_config.analog.channel_1a.is_enabled = true;
	test_config.analog.channel_1a.itr_ratio = 1;
	test_config.analog.channel_1a.type = CURRENT;
	
	test_config.analog.channel_2a.adc_to_sec_ratio = 1;
	test_config.analog.channel_2a.filter = DFT;
	test_config.analog.channel_2a.is_enabled = true;
	test_config.analog.channel_2a.itr_ratio = 1;
	test_config.analog.channel_2a.type = CURRENT;
	
	test_config.analog.channel_4a.adc_to_sec_ratio = 1;
	test_config.analog.channel_4a.filter = DFT;
	test_config.analog.channel_4a.is_enabled = true;
	test_config.analog.channel_4a.itr_ratio = 1;
	test_config.analog.channel_4a.type = CURRENT;
	
	test_config.analog.channel_5a.adc_to_sec_ratio = 1;
	test_config.analog.channel_5a.filter = DFT;
	test_config.analog.channel_5a.is_enabled = true;
	test_config.analog.channel_5a.itr_ratio = 1;
	test_config.analog.channel_5a.type = CURRENT;
	
	test_config.analog.channel_6a.adc_to_sec_ratio = 1;
	test_config.analog.channel_6a.filter = DFT;
	test_config.analog.channel_6a.is_enabled = true;
	test_config.analog.channel_6a.itr_ratio = 1;
	test_config.analog.channel_6a.type = CURRENT;
	
	ansi87t.pkp = 0.3;
	ansi87t.point_slp2 = 3;
	ansi87t.power = 100;
	ansi87t.slp1 = 0.25;
	ansi87t.slp2 = 0.75;
	ansi87t.unrestrained_pkp = 8;
	ansi87t.voltage_wd1 = 440;
	ansi87t.voltage_wd2 = 138;
	
	ANSI87T_Init(&ansi87t, 440, 138, 100, 30, 10, 0.3, 8, 0.25, 0.75);
	
	config_set(&test_config);
	
	real_time_task_handle = osThreadNew(real_time_task, NULL, &real_time_task_attributes);
	
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
}

void real_time_task(void *argument)
{
	current ansi87t_current = { 0 };
	
	adc_setup();
	
	// Pulse GPIO to start the first conversion
	GPIOD->BSRR = GPIO_PIN_12;
	GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
	
	while (1)
	{
//		real_time_take();
		
		// Wait for the conversion complete signal (set by GPIO ISR)
		osThreadFlagsWait(ADC_BUSY_FLAG, osFlagsWaitAny, osWaitForever);
		
		HAL_GPIO_WritePin(PROCESSING_TIMING_GPIO_Port, PROCESSING_TIMING_Pin, GPIO_PIN_SET);
		// Read conversion results
		ads8686s_read_channels(&ads8686s, conversion_buffer);
		
		// Do stuff here as needed
		execute_signal_processing();
		
		signal_processing_real_get(CHANNEL_0A, 1, &(ansi87t_current.current_wd1[0].real));
		signal_processing_imag_get(CHANNEL_0A, 1, &(ansi87t_current.current_wd1[0].imag));
		signal_processing_real_get(CHANNEL_1A, 1, &(ansi87t_current.current_wd1[1].real));
		signal_processing_imag_get(CHANNEL_1A, 1, &(ansi87t_current.current_wd1[1].imag));
		signal_processing_real_get(CHANNEL_2A, 1, &(ansi87t_current.current_wd1[2].real));
		signal_processing_imag_get(CHANNEL_2A, 1, &(ansi87t_current.current_wd1[2].imag));
		
		signal_processing_real_get(CHANNEL_4A, 1, &(ansi87t_current.current_wd2[0].real));
		signal_processing_imag_get(CHANNEL_4A, 1, &(ansi87t_current.current_wd2[0].imag));
		signal_processing_real_get(CHANNEL_5A, 1, &(ansi87t_current.current_wd2[1].real));
		signal_processing_imag_get(CHANNEL_5A, 1, &(ansi87t_current.current_wd2[1].imag));
		signal_processing_real_get(CHANNEL_6A, 1, &(ansi87t_current.current_wd2[2].real));
		signal_processing_imag_get(CHANNEL_6A, 1, &(ansi87t_current.current_wd2[2].imag));
		
		ANSI87T_Currents_Init(&ansi87t, &ansi87t_current);
		ANSI87T_Step(&ansi87t);
		
		if (ansi87t.trip[0] || ansi87t.trip[1] || ansi87t.trip[2])
		{
			HAL_GPIO_WritePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin, GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin, GPIO_PIN_RESET);
		}
		
		generate_and_send_magnitude_string();
		
		// Pulse GPIO to start the next conversion
		GPIOD->BSRR = GPIO_PIN_12;
		GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
		
		HAL_GPIO_WritePin(PROCESSING_TIMING_GPIO_Port, PROCESSING_TIMING_Pin, GPIO_PIN_RESET);
		
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
		signal_processing_step((uint8_t)(2 * i), ((int16_t)conversion_buffer[i].channel_a * ads8686s.lsb));
		signal_processing_step((uint8_t)(2 * i + 1), ((int16_t)conversion_buffer[i].channel_b * ads8686s.lsb));
	}
}