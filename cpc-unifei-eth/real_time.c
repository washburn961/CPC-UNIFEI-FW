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
#include "ANSI21.h"
#include "ANSI50.h"
#include "ANSI51.h"
#include "ANSI87B.h"
#include "udp_server.h"

#define SAMPLING_RATE_CONTROL_FLAG 1
#define ADC_BUSY_FLAG 2
#define GOOSE_TASK_FLAG 3
#define MAX_STRING_SIZE 512  // Adjust size as needed


ANSI87T ansi87t;

// ANSI21 related stuff
ANSI21_Config ansi21_config;
ANSI21_Current ansi21_current;
ANSI21_Voltage ansi21_voltage;
ANSI21_Mp ansi21_mp;
ANSI21_Mg ansi21_mg;
ANSI21_Qg ansi21_qg;
complex_t zlt1 = { 5.82,72.60 };
complex_t zlt0 = { 79.30,332.52 };


//ANSI 50 e 51 structs
ANSI50 ansi50;
ANSI51 ansi51;

//ANSI 87B struct
ANSI87B ansi87b;



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
void generate_and_send_magnitude_string(void);

void real_time_init(void)
{
	
	//ansi21  ansi50  ansi51
	
	
//		test_config.header.magic_number = CONFIG_MAGIC_NUMBER;
//		test_config.header.uid = 0xdeadbeee;
//		test_config.header.version = CONFIG_VERSION;
//		
//		test_config.analog.channel_0a.adc_to_sec_ratio = 10;
//		test_config.analog.channel_0a.filter = DFT;
//		test_config.analog.channel_0a.is_enabled = true;
//		test_config.analog.channel_0a.itr_ratio = 1;
//		test_config.analog.channel_0a.type = CURRENT;
//		
//		test_config.analog.channel_1a.adc_to_sec_ratio = 10;
//		test_config.analog.channel_1a.filter = DFT;
//		test_config.analog.channel_1a.is_enabled = true;
//		test_config.analog.channel_1a.itr_ratio = 1;
//		test_config.analog.channel_1a.type = CURRENT;
//		
//		test_config.analog.channel_2a.adc_to_sec_ratio = 10;
//		test_config.analog.channel_2a.filter = DFT;
//		test_config.analog.channel_2a.is_enabled = true;
//		test_config.analog.channel_2a.itr_ratio = 1;
//		test_config.analog.channel_2a.type = CURRENT;
//		
//		test_config.analog.channel_4a.adc_to_sec_ratio = 10;
//		test_config.analog.channel_4a.filter = DFT;
//		test_config.analog.channel_4a.is_enabled = true;
//		test_config.analog.channel_4a.itr_ratio = 1;
//		test_config.analog.channel_4a.type = CURRENT;
//		
//		test_config.analog.channel_5a.adc_to_sec_ratio = 10;
//		test_config.analog.channel_5a.filter = DFT;
//		test_config.analog.channel_5a.is_enabled = true;
//		test_config.analog.channel_5a.itr_ratio = 1;
//		test_config.analog.channel_5a.type = CURRENT;
//		
//		test_config.analog.channel_6a.adc_to_sec_ratio = 10;
//		test_config.analog.channel_6a.filter = DFT;
//		test_config.analog.channel_6a.is_enabled = true;
//		test_config.analog.channel_6a.itr_ratio = 1;
//		test_config.analog.channel_6a.type = CURRENT;
		
		
	
	//ansi87b
	
	
	test_config.header.magic_number = CONFIG_MAGIC_NUMBER;
	test_config.header.uid = 0xdeadbeee;
	test_config.header.version = CONFIG_VERSION;
	
	test_config.analog.channel_0a.adc_to_sec_ratio = 10;
	test_config.analog.channel_0a.filter = DFT;
	test_config.analog.channel_0a.is_enabled = true;
	test_config.analog.channel_0a.itr_ratio = 120;
	test_config.analog.channel_0a.type = CURRENT;
	
	test_config.analog.channel_1a.adc_to_sec_ratio = 10;
	test_config.analog.channel_1a.filter = DFT;
	test_config.analog.channel_1a.is_enabled = true;
	test_config.analog.channel_1a.itr_ratio = 120;
	test_config.analog.channel_1a.type = CURRENT;
	
	test_config.analog.channel_2a.adc_to_sec_ratio = 10;
	test_config.analog.channel_2a.filter = DFT;
	test_config.analog.channel_2a.is_enabled = true;
	test_config.analog.channel_2a.itr_ratio = 120;
	test_config.analog.channel_2a.type = CURRENT;
	
	test_config.analog.channel_4a.adc_to_sec_ratio = 10;
	test_config.analog.channel_4a.filter = DFT;
	test_config.analog.channel_4a.is_enabled = true;
	test_config.analog.channel_4a.itr_ratio = 300;
	test_config.analog.channel_4a.type = CURRENT;
	
	test_config.analog.channel_5a.adc_to_sec_ratio = 10;
	test_config.analog.channel_5a.filter = DFT;
	test_config.analog.channel_5a.is_enabled = true;
	test_config.analog.channel_5a.itr_ratio = 300;
	test_config.analog.channel_5a.type = CURRENT;
	
	test_config.analog.channel_6a.adc_to_sec_ratio = 10;
	test_config.analog.channel_6a.filter = DFT;
	test_config.analog.channel_6a.is_enabled = true;
	test_config.analog.channel_6a.itr_ratio = 300;
	test_config.analog.channel_6a.type = CURRENT;
	
	test_config.analog.channel_0b.adc_to_sec_ratio = 10;
	test_config.analog.channel_0b.filter = DFT;
	test_config.analog.channel_0b.is_enabled = true;
	test_config.analog.channel_0b.itr_ratio = 200;
	test_config.analog.channel_0b.type = CURRENT;
	
	test_config.analog.channel_1b.adc_to_sec_ratio = 10;
	test_config.analog.channel_1b.filter = DFT;
	test_config.analog.channel_1b.is_enabled = true;
	test_config.analog.channel_1b.itr_ratio = 200;
	test_config.analog.channel_1b.type = CURRENT;
								
	test_config.analog.channel_2b.adc_to_sec_ratio = 10;
	test_config.analog.channel_2b.filter = DFT;
	test_config.analog.channel_2b.is_enabled = true;
	test_config.analog.channel_2b.itr_ratio = 200;
	test_config.analog.channel_2b.type = CURRENT;
								
	test_config.analog.channel_4b.adc_to_sec_ratio = 10;
	test_config.analog.channel_4b.filter = DFT;
	test_config.analog.channel_4b.is_enabled = true;
	test_config.analog.channel_4b.itr_ratio = 30;
	test_config.analog.channel_4b.type = CURRENT;
								
	test_config.analog.channel_5b.adc_to_sec_ratio = 10;
	test_config.analog.channel_5b.filter = DFT;
	test_config.analog.channel_5b.is_enabled = true;
	test_config.analog.channel_5b.itr_ratio = 30;
	test_config.analog.channel_5b.type = CURRENT;
								
	test_config.analog.channel_6b.adc_to_sec_ratio = 10;
	test_config.analog.channel_6b.filter = DFT;
	test_config.analog.channel_6b.is_enabled = true;
	test_config.analog.channel_6b.itr_ratio = 30;
	test_config.analog.channel_6b.type = CURRENT;
	
//
//	//ansi87t
//	
//		test_config.header.magic_number = CONFIG_MAGIC_NUMBER;
//		test_config.header.uid = 0xdeadbeee;
//		test_config.header.version = CONFIG_VERSION;
//		
//		test_config.analog.channel_0b.adc_to_sec_ratio = 10;
//		test_config.analog.channel_0b.filter = DFT;
//		test_config.analog.channel_0b.is_enabled = true;
//		test_config.analog.channel_0b.itr_ratio = 30;
//		test_config.analog.channel_0b.type = CURRENT;
//		
//		test_config.analog.channel_1b.adc_to_sec_ratio = 10;
//		test_config.analog.channel_1b.filter = DFT;
//		test_config.analog.channel_1b.is_enabled = true;
//		test_config.analog.channel_1b.itr_ratio = 30;
//		test_config.analog.channel_1b.type = CURRENT;
//		
//		test_config.analog.channel_2b.adc_to_sec_ratio = 10;
//		test_config.analog.channel_2b.filter = DFT;
//		test_config.analog.channel_2b.is_enabled = true;
//		test_config.analog.channel_2b.itr_ratio = 30;
//		test_config.analog.channel_2b.type = CURRENT;
//		
//		test_config.analog.channel_4b.adc_to_sec_ratio = 10;
//		test_config.analog.channel_4b.filter = DFT;
//		test_config.analog.channel_4b.is_enabled = true;
//		test_config.analog.channel_4b.itr_ratio = 100;
//		test_config.analog.channel_4b.type = CURRENT;
//		
//		test_config.analog.channel_5b.adc_to_sec_ratio = 10;
//		test_config.analog.channel_5b.filter = DFT;
//		test_config.analog.channel_5b.is_enabled = true;
//		test_config.analog.channel_5b.itr_ratio = 100;
//		test_config.analog.channel_5b.type = CURRENT;
//		
//		test_config.analog.channel_6b.adc_to_sec_ratio = 10;
//		test_config.analog.channel_6b.filter = DFT;
//		test_config.analog.channel_6b.is_enabled = true;
//		test_config.analog.channel_6b.itr_ratio = 100;
//		test_config.analog.channel_6b.type = CURRENT;
	
	
	
	//ANSI21 parametrização
//	ANSI21_Init_param(&ansi21_config, zlt1, zlt0, 5.0, 5.5, 440000.0 / 115.0, 1000.0 / 5.0, 1.0 / 960.0, true, true, true, true, true, true);
//	ANSI21_Init_Mp(&ansi21_mp, 0.85, 1.15, 0.3);
//	ANSI21_Init_Mg(&ansi21_mg, 0.85, 1.15, 0.3);
//	ANSI21_Init_Qg(&ansi21_qg, 0.85, 1.15, 15.0, 20.0, 0.35);
//	
	
	//ANSI50 parametrização
//	ANSI50_Init(&ansi50, 17.0, zlt1, zlt0, 5.0, 440000.0 / 115.0, 1000.0 / 5.0, true, true, true);

	//ANSI51 parametrização
//	ANSI51_Init(&ansi51, 0.5, 5.5, 1041.67e-6, STANDARD_2, 1, zlt1, zlt0, 5.0, 440000.0 / 115.0, 1000.0 / 5.0, true, true, true);
	
	
	//ANSI87B parametrização
	ANSI87B_Init(&ansi87b, 2000, 10000, 20000, 0.2, 0.5, 4);
	
	
	//ANSI87T parametrização
//	ANSI87T_Init(&ansi87t, 440, 138, 100, 30, 100, 0.3, 8, 0.25, 0.75);
	
	
	config_set(&test_config);
	
	real_time_task_handle = osThreadNew(real_time_task, NULL, &real_time_task_attributes);
	
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
}

void real_time_task(void *argument)
{
	current ansi87t_current = { 0 };
	
	//ansi21  ansi50  ansi51
//	complex_t IphA;
//	complex_t IphB;
//	complex_t IphC;
//	complex_t VphA;
//	complex_t VphB;
//	complex_t VphC;
//	
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
		
		GPIOD->BSRR = GPIO_PIN_12;
		GPIOD->BSRR = (uint32_t)GPIO_PIN_12 << 16;
		
		// Do stuff here as needed
		execute_signal_processing();
		
		//ansi21  ansi50  ansi51
			
//		signal_processing_real_get(CHANNEL_0A, 1, &(IphA.real));
//		signal_processing_imag_get(CHANNEL_0A, 1, &(IphA.imag));
//		signal_processing_real_get(CHANNEL_1A, 1, &(IphB.real));
//		signal_processing_imag_get(CHANNEL_1A, 1, &(IphB.imag));
//		signal_processing_real_get(CHANNEL_2A, 1, &(IphC.real));
//		signal_processing_imag_get(CHANNEL_2A, 1, &(IphC.imag));
//		
//		signal_processing_real_get(CHANNEL_4A, 1, &(VphA.real));
//		signal_processing_imag_get(CHANNEL_4A, 1, &(VphA.imag));
//		signal_processing_real_get(CHANNEL_5A, 1, &(VphB.real));
//		signal_processing_imag_get(CHANNEL_5A, 1, &(VphB.imag));
//		signal_processing_real_get(CHANNEL_6A, 1, &(VphC.real));
//		signal_processing_imag_get(CHANNEL_6A, 1, &(VphC.imag));
		
		
		//ansi87b
		
		signal_processing_real_get(CHANNEL_0A, 1, &(ansi87b.current[0][0]));
		signal_processing_imag_get(CHANNEL_0A, 1, &(ansi87b.current[0][1]));
		signal_processing_real_get(CHANNEL_1A, 1, &(ansi87b.current[0][2]));
		signal_processing_imag_get(CHANNEL_1A, 1, &(ansi87b.current[0][3]));
		signal_processing_real_get(CHANNEL_2A, 1, &(ansi87b.current[0][4]));
		signal_processing_imag_get(CHANNEL_2A, 1, &(ansi87b.current[0][5]));
		
		signal_processing_real_get(CHANNEL_4A, 1, &(ansi87b.current[1][0]));
		signal_processing_imag_get(CHANNEL_4A, 1, &(ansi87b.current[1][1]));
		signal_processing_real_get(CHANNEL_5A, 1, &(ansi87b.current[1][2]));
		signal_processing_imag_get(CHANNEL_5A, 1, &(ansi87b.current[1][3]));
		signal_processing_real_get(CHANNEL_6A, 1, &(ansi87b.current[1][4]));
		signal_processing_imag_get(CHANNEL_6A, 1, &(ansi87b.current[1][5]));
		
		signal_processing_real_get(CHANNEL_4B, 1, &(ansi87b.current[2][0]));
		signal_processing_imag_get(CHANNEL_4B, 1, &(ansi87b.current[2][1]));
		signal_processing_real_get(CHANNEL_5B, 1, &(ansi87b.current[2][2]));
		signal_processing_imag_get(CHANNEL_5B, 1, &(ansi87b.current[2][3]));
		signal_processing_real_get(CHANNEL_6B, 1, &(ansi87b.current[2][4]));
		signal_processing_imag_get(CHANNEL_6B, 1, &(ansi87b.current[2][5]));
		
		signal_processing_real_get(CHANNEL_0B, 1, &(ansi87b.current[3][0]));
		signal_processing_imag_get(CHANNEL_0B, 1, &(ansi87b.current[3][1]));
		signal_processing_real_get(CHANNEL_1B, 1, &(ansi87b.current[3][2]));
		signal_processing_imag_get(CHANNEL_1B, 1, &(ansi87b.current[3][3]));
		signal_processing_real_get(CHANNEL_2B, 1, &(ansi87b.current[3][4]));
		signal_processing_imag_get(CHANNEL_2B, 1, &(ansi87b.current[3][5]));
		
		
		//ansi87t
//		
//		signal_processing_real_get(CHANNEL_0A, 1, &(ansi87t_current.current_wd1[0].real));
//		signal_processing_imag_get(CHANNEL_0A, 1, &(ansi87t_current.current_wd1[0].imag));
//		signal_processing_real_get(CHANNEL_1A, 1, &(ansi87t_current.current_wd1[1].real));
//		signal_processing_imag_get(CHANNEL_1A, 1, &(ansi87t_current.current_wd1[1].imag));
//		signal_processing_real_get(CHANNEL_2A, 1, &(ansi87t_current.current_wd1[2].real));
//		signal_processing_imag_get(CHANNEL_2A, 1, &(ansi87t_current.current_wd1[2].imag));
//		
//		signal_processing_real_get(CHANNEL_4A, 1, &(ansi87t_current.current_wd2[0].real));
//		signal_processing_imag_get(CHANNEL_4A, 1, &(ansi87t_current.current_wd2[0].imag));
//		signal_processing_real_get(CHANNEL_5A, 1, &(ansi87t_current.current_wd2[1].real));
//		signal_processing_imag_get(CHANNEL_5A, 1, &(ansi87t_current.current_wd2[1].imag));
//		signal_processing_real_get(CHANNEL_6A, 1, &(ansi87t_current.current_wd2[2].real));
//		signal_processing_imag_get(CHANNEL_6A, 1, &(ansi87t_current.current_wd2[2].imag));
		

		
			
		//ANSI 21 sets
//		ANSI21_Set_current(&ansi21_current, IphA, IphB, IphC);
//		ANSI21_Set_voltage(&ansi21_voltage, VphA, VphB, VphC);
//		
		//ANSI21 steps
//		ANSI21_Step_Mp(&ansi21_mp, &ansi21_mg, &ansi21_qg, &ansi21_current, &ansi21_voltage, &ansi21_config);
//		ANSI21_Step_Mg(&ansi21_mp, &ansi21_mg, &ansi21_qg, &ansi21_current, &ansi21_voltage, &ansi21_config);
//		ANSI21_Step_Qg(&ansi21_mp, &ansi21_mg, &ansi21_qg, &ansi21_current, &ansi21_voltage, &ansi21_config);
		
		
		//ANSI50 set
//		ANSI50_SetCurrentVoltage(&ansi50, IphA, IphB, IphC, VphA, VphB, VphC);
		//ANSI50 step
//		ANSI50_Step(&ansi50);
		
		//ANSI51 set
//		ANSI51_SetCurrentVoltage(&ansi51, IphA, IphB, IphC, VphA, VphB, VphC);
		//ANSI51 step
//		ANSI51_Step(&ansi51);
				
		
		//ANSI87B step
		ANSI87B_Step(&ansi87b);
		
		
		//ANSI87T step
//		ANSI87T_Currents_Init(&ansi87t, &ansi87t_current);
//		ANSI87T_Step(&ansi87t);
		
		
		//ansi21
		
//		if (ansi21_mp.is_tripped[0] || ansi21_mp.is_tripped[1] || ansi21_mp.is_tripped[2])
//		{
//			HAL_GPIO_WritePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin, GPIO_PIN_SET);
//		}
//		else
//		{
//			HAL_GPIO_WritePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin, GPIO_PIN_RESET);
//		}
//		
//		if (ansi21_mg.is_tripped[0] || ansi21_mg.is_tripped[1] || ansi21_mg.is_tripped[2])
//		{
//			HAL_GPIO_WritePin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin, GPIO_PIN_SET);
//		}
//		else
//		{
//			HAL_GPIO_WritePin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin, GPIO_PIN_RESET);
//		}
//		
//		if (ansi21_qg.is_tripped[0] || ansi21_qg.is_tripped[1] || ansi21_qg.is_tripped[2])
//		{
//			HAL_GPIO_WritePin(OUT2_A_OUT_GPIO_Port, OUT2_A_OUT_Pin, GPIO_PIN_SET);
//		}
//		else
//		{
//			HAL_GPIO_WritePin(OUT2_A_OUT_GPIO_Port, OUT2_A_OUT_Pin, GPIO_PIN_RESET);
//		}
//		
//		
//		//ansi50
//		
//		if (ansi50.is_tripped[0] || ansi50.is_tripped[1] || ansi50.is_tripped[2])
//		{
//			HAL_GPIO_WritePin(OUT4_A_OUT_GPIO_Port, OUT4_A_OUT_Pin, GPIO_PIN_SET);
//		}
//		else
//		{
//			HAL_GPIO_WritePin(OUT4_A_OUT_GPIO_Port, OUT4_A_OUT_Pin, GPIO_PIN_RESET);
//		}
//		
//		
//		//ansi51
//		
//		if (ansi51.is_tripped[0] || ansi51.is_tripped[1] || ansi51.is_tripped[2])
//		{
//			HAL_GPIO_WritePin(OUT1_B_OUT_GPIO_Port, OUT1_B_OUT_Pin, GPIO_PIN_SET);
//		}
//		else
//		{
//			HAL_GPIO_WritePin(OUT1_B_OUT_GPIO_Port, OUT1_B_OUT_Pin, GPIO_PIN_RESET);
//		}
//		
//		if (ansi51.is_pickupped[0] || ansi51.is_pickupped[1] || ansi51.is_pickupped[2])
//		{
//			HAL_GPIO_WritePin(OUT3_B_OUT_GPIO_Port, OUT3_B_OUT_Pin, GPIO_PIN_SET);
//		}
//		else
//		{
//			HAL_GPIO_WritePin(OUT3_B_OUT_GPIO_Port, OUT3_B_OUT_Pin, GPIO_PIN_RESET);
//		}
//		
		
		//ansi87b - ARRUMAR AS VARIÀVEIS
		
		if (ansi87t.trip[0] || ansi87t.trip[1] || ansi87t.trip[2])
		{
			HAL_GPIO_WritePin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin, GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(OUT1_A_OUT_GPIO_Port, OUT1_A_OUT_Pin, GPIO_PIN_RESET);
		}
		
		//ansi87t
		
//		if (ansi87t.trip[0] || ansi87t.trip[1] || ansi87t.trip[2])
//		{
//			HAL_GPIO_WritePin(OUT4_B_OUT_GPIO_Port, OUT4_B_OUT_Pin, GPIO_PIN_SET);
//		}
//		else
//		{
//			HAL_GPIO_WritePin(OUT4_B_OUT_GPIO_Port, OUT4_B_OUT_Pin, GPIO_PIN_RESET);
//		}
//		
//		generate_and_send_magnitude_string();
//		
//		HAL_GPIO_WritePin(PROCESSING_TIMING_GPIO_Port, PROCESSING_TIMING_Pin, GPIO_PIN_RESET);
//		
//		osThreadFlagsWait(SAMPLING_RATE_CONTROL_FLAG, osFlagsWaitAny, osWaitForever);
		
		// Pulse GPIO to start the next conversion
		
//		real_time_release();
	}
}

void adc_setup(void)
{
	struct ads8686s_init_param ads8686s_init_param = {
		.osr = ADS8686S_OSR_2
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


char message[MAX_STRING_SIZE];
// Collect all the channels from 0A to 7A and 0B to 7B
const uint8_t channels_to_process[] = {
	CHANNEL_0A,
	CHANNEL_1A,
	CHANNEL_2A,
	CHANNEL_3A,
	CHANNEL_4A,
	CHANNEL_5A,
	CHANNEL_6A,
	CHANNEL_7A,
	CHANNEL_0B,
	CHANNEL_1B,
	CHANNEL_2B,
	CHANNEL_3B,
	CHANNEL_4B,
	CHANNEL_5B,
	CHANNEL_6B,
	CHANNEL_7B
};
float magnitude[16] = { 0 };
float phase[16] = { 0 };
void generate_and_send_magnitude_string(void)
{
	// Process each channel for magnitude and phase
	for (int i = 0; i < 16; i++) {
		signal_processing_mag_get(channels_to_process[i], 1, &(magnitude[i]));
		signal_processing_phase_get(channels_to_process[i], 1, &(phase[i]));
	}

	// Format the message string with the results
	size_t len = sprintf(message,
		"%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,"
		"%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,"
		"%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,"
		"%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\r\n",
		magnitude[0],
		phase[0],
		magnitude[1],
		phase[1],
		magnitude[2],
		phase[2],
		magnitude[3],
		phase[3],
		magnitude[4],
		phase[4],
		magnitude[5],
		phase[5],
		magnitude[6],
		phase[6],
		magnitude[7],
		phase[7],
		magnitude[8],
		phase[8],
		magnitude[9],
		phase[9],
		magnitude[10],
		phase[10],
		magnitude[11],
		phase[11],
		magnitude[12],
		phase[12],
		magnitude[13],
		phase[13],
		magnitude[14],
		phase[14],
		magnitude[15],
		phase[15]);

	udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, message, len);
}
