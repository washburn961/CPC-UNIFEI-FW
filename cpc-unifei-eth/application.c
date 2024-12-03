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
#include "nor_flash.h"
#include "goose_frame.h"
#include "goose_publisher.h"
#include "goose.h"

void application_task(void *argument);
void goose_task(void *argument);
void link_output(uint8_t* byte_stream, size_t length);

osThreadId_t application_task_handle;
const osThreadAttr_t application_task_attributes = {
	.name = "application_task",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t goose_task_handle;
const osThreadAttr_t goose_task_attributes = {
	.name = "goose_task",
	.stack_size = 512 * 4,
	.priority = (osPriority_t) osPriorityNormal,
};

void application_init(void)
{
	application_task_handle = osThreadNew(application_task, NULL, &application_task_attributes);
	goose_task_handle = osThreadNew(goose_task, NULL, &goose_task_attributes);
}

void application_task(void *argument)
{
	while (true)
	{
		HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin);
		osDelay(500);
	}
}

goose_message_params input_mon_goose_params;
goose_handle* input_mon_goose_handle;

void goose_task(void* argument)
{
	uint8_t button_status = 0x0;
	uint8_t prev_button_status = 0x0;
	
	goose_publisher_init(&link_output);
	input_mon_goose_handle = goose_init(gnetif.hwaddr, destination, app_id);
	
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
		
	for (size_t i = 0; i < 1; i++)
	{
		goose_all_data_entry_add(input_mon_goose_handle, 0x83, sizeof(button_status), &(button_status));
	}
	
	input_mon_goose_params.default_time_allowed_to_live = 1000;
	input_mon_goose_params.name = "inputMonitoring";
	input_mon_goose_params.handle = input_mon_goose_handle;
	
	goose_publisher_register(input_mon_goose_params);
	
	while (true)
	{
		button_status = (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET) ? 0x01 : 0x00;
		
		if (button_status != prev_button_status)
		{
			goose_all_data_entry_modify(input_mon_goose_handle, 0, 0x83, sizeof(button_status), &(button_status));
			goose_publisher_notify(input_mon_goose_params.name);
		}
		
		prev_button_status = button_status;
		
		goose_publisher_process();
		osDelay(1);
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
		printf("ERRO");
	}

	// Free the pbuf
	pbuf_free(p);
    
	UNLOCK_TCPIP_CORE();
}