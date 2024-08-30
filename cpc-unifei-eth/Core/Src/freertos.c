/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* ETH_CODE: add lwiperf, see comment in StartDefaultTask function */
#include "lwip.h"
#include "udp_server.h"
#include "adc.h"
#include <string.h>
#include "octospi.h"
#include "ads8686s.h"
#include "command_system.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
struct ads8686s_device dev;
uint16_t ret = 0;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osThreadId_t temperatureTaskHandle;
const osThreadAttr_t temperatureTask_attributes = {
	.name = "temperatureTask",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};

void TemperatureTask(void *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
	   /* ETH_CODE: add breakpoint when stack oveflow is detected by FreeRTOS.
	* Useful for debugging issues.
	*/
	__BKPT(0);
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	defaultTaskHandle = osThreadNew(TemperatureTask, NULL, &temperatureTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */

/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
/* ETH_CODE: Adding lwiperf to measure TCP/IP performance.
     * iperf 2.0.6 (or older?) is required for the tests. Newer iperf2 versions
     * might work without data check, but they send different headers.
     * iperf3 is not compatible at all.
     * Adding lwiperf.c file to the project is necessary.
     * The default include path should already contain
     * 'lwip/apps/lwiperf.h'
     */
	udp_server_init();
	
	ads8686s_setup(&dev);
	ads8686s_read(&dev, 0x2, &ret);
	ads8686s_read(&dev, 0x3, &ret);
	ads8686s_read(&dev, 0x4, &ret);
	ads8686s_read(&dev, 0x5, &ret);
	ads8686s_read(&dev, 0x6, &ret);
	ads8686s_read(&dev, 0x7, &ret);
	ads8686s_read(&dev, 0x8, &ret);

	/* Infinite loop */
	for (;;)
	{
		HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
		osDelay(250);
		HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
		HAL_GPIO_TogglePin(USER_LED1_GPIO_Port, USER_LED1_Pin);
		osDelay(250);
		HAL_GPIO_TogglePin(USER_LED1_GPIO_Port, USER_LED1_Pin);
	}
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* Function to send the temperature over UDP */
void SendTemperatureUDP(float temperature)
{
	char message[50];
	struct pbuf *p;
	ip_addr_t dest_ip;
	IP4_ADDR(&dest_ip, 192, 168, 1, 11);

	// Format the temperature as a string with "Celsius"
	snprintf(message, sizeof(message), "Temperature: %.2f Celsius", temperature);

	// Allocate a pbuf for the message
	udp_server_send(IPV4_ADDR(192, 168, 1, 11), 80, message, sizeof(message));
}

uint32_t temp_counts = 0;
float temp = 0;
uint16_t *TS_CAL1 = (uint16_t *)0x1FF1E820;
uint16_t *TS_CAL2 = (uint16_t *)0x1FF1E840;
	
void TemperatureTask(void *argument)
{
	HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
	
	for (;;)
	{
		HAL_ADC_Start(&hadc3);
		HAL_ADC_PollForConversion(&hadc3, 100);
		temp = ((double)(80.0)) / ((double)(*TS_CAL2 - *TS_CAL1)) * (((double)(HAL_ADC_GetValue(&hadc3))) - ((double)*TS_CAL1)) + ((double)30.0);
		HAL_ADC_Stop(&hadc3);
		
		// Send the temperature over UDP
		SendTemperatureUDP(temp);
		
		osDelay(500);
	}
}
/* USER CODE END Application */

