#include "application.h"

#define SAMPLING_RATE_CONTROL_FLAG 0x0001
#define ADC_BUSY_FLAG 0x0002
#define GOOSE_TASK_FLAG 0x0003
#define CHANNEL_COUNT 16
#define SAMPLE_COUNT 16
#define VTOC_RATIO 10.0f
#define CTR 4000.0f / 5.0f
#define VTOC(v) ((v) * VTOC_RATIO)
#define SEC2PRI(i) ((i) * CTR)


void AnalogTask(void *argument);
void BlinkTask(void *argument);
void serialize_voltages(struct ads8686s_conversion_voltage *voltages, uint8_t* out_serialized, uint8_t size);
void link_output(uint8_t* byte_stream, size_t length);
void parametrize_protection(void);
void send_float_over_udp(float value);

ANSI51 ansi51_element;
ANSI50 ansi50_element;
ANSI87B ansi87b_element;
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
ring_buffer ring_buffer_arr[CHANNEL_COUNT];
fourier_transform fourier_transform_arr[CHANNEL_COUNT];
float ring_buffer_content_arr[CHANNEL_COUNT][SAMPLE_COUNT] = { 0 };
float real_arr[CHANNEL_COUNT][SAMPLE_COUNT] = { 0 };
float imag_arr[CHANNEL_COUNT][SAMPLE_COUNT] = { 0 };
float sin_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };
float cos_coef[SAMPLE_COUNT * SAMPLE_COUNT] = { 0 };
float ring_buffer_tmp[SAMPLE_COUNT] = { 0 };
size_t tick_ms = 0;

uint32_t application_init(void)
{
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
	
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_Base_Start_IT(&htim3);
	
	return 0;
}

void AnalogTask(void *argument)
{
	parametrize_protection();
	
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
	size_t output_delay = 0;
	GPIO_PinState prev_pin_state = GPIO_PIN_RESET;
	GPIO_PinState pin_state = GPIO_PIN_RESET;
	float temp = 0;
	
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
		send_float_over_udp(SEC2PRI(VTOC(temp))) ;
		
		ansi87b_element.current[0][0] = SEC2PRI(VTOC(real_arr[0][1]));
		ansi87b_element.current[0][1] = SEC2PRI(VTOC(imag_arr[0][1]));
		ansi87b_element.current[0][2] = SEC2PRI(VTOC(real_arr[2][1]));
		ansi87b_element.current[0][3] = SEC2PRI(VTOC(imag_arr[2][1]));
		ansi87b_element.current[0][4] = SEC2PRI(VTOC(real_arr[4][1]));
		ansi87b_element.current[0][5] = SEC2PRI(VTOC(imag_arr[4][1]));
		
		ansi87b_element.current[1][0] = SEC2PRI(VTOC(real_arr[8][1]));
		ansi87b_element.current[1][1] = SEC2PRI(VTOC(imag_arr[8][1]));
		ansi87b_element.current[1][2] = SEC2PRI(VTOC(real_arr[10][1]));
		ansi87b_element.current[1][3] = SEC2PRI(VTOC(imag_arr[10][1]));
		ansi87b_element.current[1][4] = SEC2PRI(VTOC(real_arr[12][1]));
		ansi87b_element.current[1][5] = SEC2PRI(VTOC(imag_arr[12][1]));
		
		ansi87b_element.current[2][0] = SEC2PRI(VTOC(real_arr[1][1]));
		ansi87b_element.current[2][1] = SEC2PRI(VTOC(imag_arr[1][1]));
		ansi87b_element.current[2][2] = SEC2PRI(VTOC(real_arr[3][1]));
		ansi87b_element.current[2][3] = SEC2PRI(VTOC(imag_arr[3][1]));
		ansi87b_element.current[2][4] = SEC2PRI(VTOC(real_arr[5][1]));
		ansi87b_element.current[2][5] = SEC2PRI(VTOC(imag_arr[5][1]));
		
		ansi87b_element.current[3][0] = SEC2PRI(VTOC(real_arr[9][1]));
		ansi87b_element.current[3][1] = SEC2PRI(VTOC(imag_arr[9][1]));
		ansi87b_element.current[3][2] = SEC2PRI(VTOC(real_arr[11][1]));
		ansi87b_element.current[3][3] = SEC2PRI(VTOC(imag_arr[11][1]));
		ansi87b_element.current[3][4] = SEC2PRI(VTOC(real_arr[13][1]));
		ansi87b_element.current[3][5] = SEC2PRI(VTOC(imag_arr[13][1]));
		
		HAL_GPIO_WritePin(PROCESSING_TIMING_GPIO_Port, PROCESSING_TIMING_Pin, GPIO_PIN_SET);
		ANSI87B_Step(&ansi87b_element);
		HAL_GPIO_WritePin(PROCESSING_TIMING_GPIO_Port, PROCESSING_TIMING_Pin, GPIO_PIN_RESET);
		
		pin_state = ansi87b_element.trip ? GPIO_PIN_SET : GPIO_PIN_RESET;
		
		if (ansi87b_element.trip)
		{
			HAL_GPIO_WritePin(USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(USER_LED1_GPIO_Port, USER_LED1_Pin, GPIO_PIN_RESET);
		}
		
		if (output_delay == 0)
		{
			if (prev_pin_state != pin_state)
			{
				HAL_GPIO_WritePin(OUT3_A_OUT_GPIO_Port, OUT3_A_OUT_Pin, pin_state);
				prev_pin_state = pin_state;
				output_delay = 96;
			}
		}
		else
		{
			output_delay--;
		}
		
		// Wait for the timer interrupt to control the sampling rate
		osThreadFlagsWait(SAMPLING_RATE_CONTROL_FLAG, osFlagsWaitAny, osWaitForever);
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

// ISR or other function where the analog conversion is complete
void application_adc_timing_flag_set(void)
{
	osThreadFlagsSet(analogTaskHandle, SAMPLING_RATE_CONTROL_FLAG); // Signal to start conversion
}

void application_adc_busy_flag_set(void)
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

void parametrize_protection()
{
	ANSI87B_Init(&ansi87b_element, 2e3f, 9e3f, 25e3f, 0.2f, 0.5f);
//	ANSI51_Init(&ansi51_element, 0.5f, 5.5f, 0.00104166666f, STANDARD_2, 1);
//	ANSI50_Init(&ansi50_element, 17);
}

void send_float_over_udp(float value)
{
	char buffer[32]; // Create a buffer to hold the string representation of the float

	// Convert the float to a string with 8 decimal places
	snprintf(buffer, sizeof(buffer), "%.8f", value);

	// Send the string over UDP using the default address and port
	udp_server_send(DEFAULT_IPV4_ADDR, DEFAULT_PORT, buffer, (uint16_t)strlen(buffer));
}