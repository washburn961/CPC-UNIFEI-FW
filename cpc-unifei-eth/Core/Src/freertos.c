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
#include "udp.h"
#include "lwip/apps/lwiperf.h"
#include "adc.h"
#include <string.h>
#include "octospi.h"
#include "ads8686s_interface.h"
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
	ADS8686S_Interface_Init();
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
struct udp_pcb *udp_pcb_server;

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	static uint8_t counter = 0;
	if (p != NULL)
	{
		// Send a response back to the sender (optional)
		struct pbuf *reply = pbuf_alloc(PBUF_TRANSPORT, p->len, PBUF_RAM);
		if (reply != NULL)
		{
			memcpy(reply->payload, p->payload, p->len);
			udp_sendto(pcb, reply, addr, port);
			if (counter++ == 0)
			{
				HAL_GPIO_TogglePin(USER_LED2_GPIO_Port, USER_LED2_Pin);
			}
			pbuf_free(reply);
		}

		// Free the received packet buffer
		pbuf_free(p);
	}
}

void udp_server_init(void)
{
	udp_pcb_server = udp_new();
	if (udp_pcb_server != NULL)
	{
		err_t err = udp_bind(udp_pcb_server, IP_ADDR_ANY, 80); // Bind to port 80
		if (err == ERR_OK)
		{
			udp_recv(udp_pcb_server, udp_receive_callback, NULL); // Set up the receive callback
		}
	}
}

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
	LOCK_TCPIP_CORE();
	lwiperf_start_tcp_server_default(NULL, NULL);

	ip4_addr_t remote_addr;
	IP4_ADDR(&remote_addr, 192, 168, 1, 11);
	lwiperf_start_tcp_client_default(&remote_addr, NULL, NULL);
	
	// Initialize the UDP server
	udp_server_init();
	UNLOCK_TCPIP_CORE();
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

	// Format the temperature as a string with "Celsius"
	snprintf(message, sizeof(message), "Temperature: %.2f Celsius", temperature);

	// Allocate a pbuf for the message
	LOCK_TCPIP_CORE();
	p = pbuf_alloc(PBUF_TRANSPORT, strlen(message), PBUF_RAM);
	if (p != NULL)
	{
		// Copy the message into the pbuf
		memcpy(p->payload, message, strlen(message));

		// Send the pbuf using the UDP server PCB
		udp_sendto(udp_pcb_server, p, IP_ADDR_BROADCAST, 80);

		// Free the pbuf
		pbuf_free(p);
	}
	UNLOCK_TCPIP_CORE();
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

